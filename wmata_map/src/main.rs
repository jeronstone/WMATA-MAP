use reqwest::header::{HeaderMap, HeaderValue};
use reqwest::Client;
use serde::{Deserialize, Serialize};
use std::env;
use std::fmt::Debug;
use std::collections::HashMap;
use std::ptr::null;
use sled::{Db, Tree};
use bincode::{serialize, deserialize};

const WMATA_URL: &str = "https://api.wmata.com";

const SLED_CACHE: &str = "std_route_cache";
const SLED_ROUTE_TREE: &str = "route_tree";
const SLED_KEY: &[u8] = b"vals";

#[derive(Clone, Serialize, Deserialize, Debug)]
struct TrackCircuit {
    SeqNum: i32,
    CircuitId: i32,
    StationCode: Option<String>
}

#[derive(Clone, Serialize, Deserialize, Debug)]
struct StandardRoute {
    LineCode: String,
    TrackNum: i32,
    TrackCircuits: Vec<TrackCircuit>
}

#[derive(Serialize, Deserialize, Debug)]
struct StdRoutesAPIResponse {
    StandardRoutes: Vec<StandardRoute>
}

#[derive(Serialize, Deserialize, Debug)]
struct TrainPosition {
    TrainId: String,
    TrainNumber: String,
    CarCount: i32,
    DirectionNum: i32,
    CircuitId: i32,
    DestinationStationCode: Option<String>,
    LineCode: Option<String>,
    SecondsAtLocation: i32,
    ServiceType: String
}

#[derive(Serialize, Deserialize, Debug)]
struct TrainPosAPIResponse {
    TrainPositions: Vec<TrainPosition>
}

#[derive(Serialize, Deserialize, Debug)]
struct Neighbor {
    NeighborType: String,
    CircuitIds: Vec<i32>
}

#[derive(Serialize, Deserialize, Debug)]
struct Circuit {
    Track: i32,
    CircuitId: i32,
    Neighbors: Vec<Neighbor>
}

#[derive(Serialize, Deserialize, Debug)]
struct CircuitAPIResponse {
    TrackCircuits: Vec<Circuit>
}

async fn make_api_call<T: Debug>(key: String, url: String) -> Result<(T), Box<dyn std::error::Error>> 
    where for<'de> T: Deserialize<'de>, {
    let client = Client::new();

    let mut headers = HeaderMap::new();
    headers.insert("Cache-Control", HeaderValue::from_str("no-cache").expect("nocache fail"));
    headers.insert("api_key", HeaderValue::from_str(&key).expect("api fail"));

    let response = client
        .get(url)
        .headers(headers)
        .send()
        .await?;

    if response.status().is_success() {
        // Deserialize the JSON response into your struct
        let api_response: T = response.json().await?;
        //println!("API Response: {:?}", api_response);
        Ok(api_response)
    } else {
        // eprintln!("API call failed with status: {}", response.status());
        // eprintln!("Response body: {}", response.text().await?);
        Err("API FAIL".into())
    }

}

async fn get_std_routes(key: String) -> Result<(StdRoutesAPIResponse), Box<dyn std::error::Error>> {
    Ok(make_api_call::<StdRoutesAPIResponse>(key, WMATA_URL.to_owned() + "/TrainPositions/StandardRoutes?contentType=json").await?)
}

async fn get_train_pos(key: String) -> Result<(TrainPosAPIResponse), Box<dyn std::error::Error>> {
    Ok(make_api_call::<TrainPosAPIResponse>(key, WMATA_URL.to_owned() + "/TrainPositions/TrainPositions?contentType=json").await?)   
}

async fn get_circuits(key: String) -> Result<(CircuitAPIResponse), Box<dyn std::error::Error>> {
    Ok(make_api_call::<CircuitAPIResponse>(key, WMATA_URL.to_owned() + "/TrainPositions/TrackCircuits?contentType=json").await?)
}

#[derive(Clone, Debug)]
enum LineDictValue {
    Route(StandardRoute),
    Int(i32),
}

type SledInnerMap = HashMap<String, f64>;
#[derive(Serialize, Deserialize, Debug)]
struct SledWrapper(HashMap<i32, SledInnerMap>);

async fn req_and_cache_std_routes(key: String) -> Result<(), Box<dyn std::error::Error>> {
    
    let mut lines: HashMap<String, HashMap<String, LineDictValue>> = HashMap::new();
    
    let std_rts = get_std_routes(key).await?;
    for r in std_rts.StandardRoutes.iter() {
        let mut line_dict : HashMap<String, LineDictValue> = HashMap::new();
        line_dict.insert(String::from("route"), LineDictValue::Route(r.clone()));
        line_dict.insert(String::from("max_seq"), LineDictValue::Int(r.TrackCircuits.last().unwrap().SeqNum));
        lines.insert(r.LineCode.clone() + "_" + &r.TrackNum.to_string(), line_dict);
    }
    
    let mut circuits: HashMap<i32, HashMap<String, f64>> = HashMap::new();
    for (key, value) in lines.iter() {
        match (value.get("route").unwrap(), value.get("max_seq").unwrap()) {
            (LineDictValue::Route(standard_route), LineDictValue::Int(max_seq)) => {
                for c in standard_route.TrackCircuits.iter() {
                    let mut cir_dict: HashMap<String, f64> = HashMap::new();
                    cir_dict.insert(String::from("seq_perc"), c.SeqNum as f64 / *max_seq as f64);
                    circuits.insert(c.CircuitId, cir_dict);
                }
            },
            _ => panic!("never happens"),
        }
    }
    
    //println!("{:?}", circuits);

    let db = sled::open(SLED_CACHE)?;
    let tree: Tree = db.open_tree(SLED_ROUTE_TREE)?;

    let wrapper = SledWrapper(circuits);
    let bytes = bincode::serialize(&wrapper)?;
    tree.insert(SLED_KEY, bytes)?;
   
    Ok(())  
}

fn read_std_routes_from_cache() -> Result<(SledWrapper), Box<dyn std::error::Error>> {

    let db: sled::Db = sled::open(SLED_CACHE).unwrap(); 

    let tree = db.open_tree(SLED_ROUTE_TREE)?;

    if let Some(val) = tree.get(SLED_KEY)? {
        let decoded: SledWrapper = bincode::deserialize(&val)?;
        println!("Decoded: {:?}", decoded);
        Ok(decoded)
    } else {
        Err("Sled read err".into())
    }
}

async fn get_curr_train_seq_percs(key: String)  -> Result<(HashMap<String, Vec<f64>>), Box<dyn std::error::Error>> {
    let train_pos = get_train_pos(key).await?;

    let circuits = read_std_routes_from_cache()?;

    let mut trains_per_line: HashMap<String, Vec<f64>> = HashMap::new();
    for t in train_pos.TrainPositions.iter() {
        match (&t.LineCode, circuits.0.get(&t.CircuitId)) {
            (Some(lc), Some(ca)) => {
                trains_per_line.entry(lc.to_string()).or_insert_with(Vec::new).push(*ca.get("seq_perc").unwrap());
            },
            _ => continue,
        }
    };

    println!("{:?}", trains_per_line);

    Ok(trains_per_line)
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {

    dotenvy::dotenv().ok();

    let wmata_api_key  = env::var("WMATA_API_KEY").expect("WMATA API key not set");
    
    get_curr_train_seq_percs(wmata_api_key).await?;

    Ok(())
}