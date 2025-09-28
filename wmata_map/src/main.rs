mod api_handle;

use std::{env, fmt::Debug, collections::HashMap};
use sled::{Tree};
use serde::{Deserialize, Serialize};
use bincode;

const SLED_CACHE: &str = "std_route_cache";
const SLED_ROUTE_TREE: &str = "route_tree";
const SLED_KEY: &[u8] = b"vals";

#[derive(Clone, Debug)]
enum LineDictValue {
    Route(api_handle::StandardRoute),
    Int(i32),
}

type SledInnerMap = HashMap<String, f64>;
#[derive(Serialize, Deserialize, Debug)]
struct SledRouteMap(HashMap<i32, SledInnerMap>);

async fn req_and_cache_std_routes(key: String) -> Result<(), Box<dyn std::error::Error>> {
    
    let mut lines: HashMap<String, HashMap<String, LineDictValue>> = HashMap::new();
    
    let std_rts = api_handle::get_std_routes(key).await?;
    for r in std_rts.StandardRoutes.iter() {
        let mut line_dict : HashMap<String, LineDictValue> = HashMap::new();
        line_dict.insert(String::from("route"), LineDictValue::Route(r.clone()));
        line_dict.insert(String::from("max_seq"), LineDictValue::Int(r.TrackCircuits.last().unwrap().SeqNum));
        lines.insert(r.LineCode.clone() + "_" + &r.TrackNum.to_string(), line_dict);
    }
    
    let mut circuits: HashMap<i32, HashMap<String, f64>> = HashMap::new();
    for (_key, value) in lines.iter() {
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

    let db = sled::open(SLED_CACHE)?;
    let tree: Tree = db.open_tree(SLED_ROUTE_TREE)?;

    let wrapper = SledRouteMap(circuits);
    let bytes = bincode::serialize(&wrapper)?;
    tree.insert(SLED_KEY, bytes)?;
   
    Ok(())  
}

fn read_std_routes_from_cache() -> Result<SledRouteMap, Box<dyn std::error::Error>> {

    let db: sled::Db = sled::open(SLED_CACHE).unwrap(); 

    let tree = db.open_tree(SLED_ROUTE_TREE)?;

    if let Some(val) = tree.get(SLED_KEY)? {
        let decoded: SledRouteMap = bincode::deserialize(&val)?;
        Ok(decoded)
    } else {
        Err("Sled read err".into())
    }
}

async fn get_curr_train_seq_percs(key: String, std_routes: SledRouteMap, draw: bool)  -> Result<HashMap<String, Vec<f64>>, Box<dyn std::error::Error>> {
    let train_pos = api_handle::get_train_pos(key).await?;

    let mut trains_per_line: HashMap<String, Vec<f64>> = HashMap::new();
    for t in train_pos.TrainPositions.iter() {
        match (&t.LineCode, std_routes.0.get(&t.CircuitId)) {
            (Some(lc), Some(ca)) => {
                trains_per_line.entry(lc.to_string()).or_insert_with(Vec::new).push(*ca.get("seq_perc").unwrap());
            },
            _ => continue,
        }
    };

    if draw {
        let w = 100;
        
        for (key, values) in &trains_per_line {
            println!("{}:", key);

            let mut line = vec!['-'; w + 1];
            line[0] = '|';
            line[w] = '|';

            for &val in values {
                let pos = (val * w as f64).round() as usize;
                if pos <= w {
                    line[pos] = 'x';
                }
            }

            let chart: String = line.into_iter().collect();
            println!("{}", chart);
        }
    }

    Ok(trains_per_line)
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {

    dotenvy::dotenv().ok();
    let wmata_api_key  = env::var("WMATA_API_KEY").expect("WMATA API key not set");

    let mut no_cache = false;

    let args: Vec<String> = env::args().collect();
    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "--no-cache" => no_cache = true,
            _ => eprintln!("Unknown argument: {}", args[i]),
        }
        i += 1;
    }
    

    if no_cache {
        println!("Getting new std routes and saving to cache...");
        req_and_cache_std_routes(wmata_api_key.clone()).await?;
    } else {
        println!("Using std routes from cache...")
    }

    let circuits: SledRouteMap = read_std_routes_from_cache()?;
    let _line_percs = get_curr_train_seq_percs(wmata_api_key.clone(), circuits, true).await?;

    Ok(())
}