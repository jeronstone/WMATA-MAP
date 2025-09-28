use reqwest::{Client, header::{HeaderMap, HeaderValue}};
use serde::{Deserialize, Serialize};
use std::fmt::Debug;

const WMATA_URL: &str = "https://api.wmata.com";

#[derive(Clone, Serialize, Deserialize, Debug)]
pub struct TrackCircuit {
    pub SeqNum: i32,
    pub CircuitId: i32,
    pub StationCode: Option<String>
}

#[derive(Clone, Serialize, Deserialize, Debug)]
pub struct StandardRoute {
    pub LineCode: String,
    pub TrackNum: i32,
    pub TrackCircuits: Vec<TrackCircuit>
}

#[derive(Serialize, Deserialize, Debug)]
pub struct StdRoutesAPIResponse {
    pub StandardRoutes: Vec<StandardRoute>
}

#[derive(Serialize, Deserialize, Debug)]
pub struct TrainPosition {
    pub TrainId: String,
    pub TrainNumber: String,
    pub CarCount: i32,
    pub DirectionNum: i32,
    pub CircuitId: i32,
    pub DestinationStationCode: Option<String>,
    pub LineCode: Option<String>,
    pub SecondsAtLocation: i32,
    pub ServiceType: String
}

#[derive(Serialize, Deserialize, Debug)]
pub struct TrainPosAPIResponse {
    pub TrainPositions: Vec<TrainPosition>
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Neighbor {
    pub NeighborType: String,
    pub CircuitIds: Vec<i32>
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Circuit {
    pub Track: i32,
    pub CircuitId: i32,
    pub Neighbors: Vec<Neighbor>
}

#[derive(Serialize, Deserialize, Debug)]
pub struct CircuitAPIResponse {
    pub TrackCircuits: Vec<Circuit>
}

pub async fn make_api_call<T: Debug>(key: String, url: String) -> Result<T, Box<dyn std::error::Error>> 
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
        let api_response: T = response.json().await?;
        Ok(api_response)
    } else {
        Err("API FAIL".into())
    }

}

pub async fn get_std_routes(key: String) -> Result<StdRoutesAPIResponse, Box<dyn std::error::Error>> {
    Ok(make_api_call::<StdRoutesAPIResponse>(key, WMATA_URL.to_owned() + "/TrainPositions/StandardRoutes?contentType=json").await?)
}

pub async fn get_train_pos(key: String) -> Result<TrainPosAPIResponse, Box<dyn std::error::Error>> {
    Ok(make_api_call::<TrainPosAPIResponse>(key, WMATA_URL.to_owned() + "/TrainPositions/TrainPositions?contentType=json").await?)   
}

pub async fn get_circuits(key: String) -> Result<CircuitAPIResponse, Box<dyn std::error::Error>> {
    Ok(make_api_call::<CircuitAPIResponse>(key, WMATA_URL.to_owned() + "/TrainPositions/TrackCircuits?contentType=json").await?)
}