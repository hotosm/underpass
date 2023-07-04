import React, { useState, useEffect, useRef } from 'react';
import { MapContainer, TileLayer, useMap, useMapEvent, Polygon, Popup, Marker } from 'react-leaflet'
import API from '../api';
import "./styles.css";
import 'leaflet/dist/leaflet.css';

function getBBoxString(map) {
  let bbox = [
    [
      map.getBounds().getNorthEast().lng,
      map.getBounds().getNorthEast().lat
    ].join(" "),
    [
      map.getBounds().getNorthWest().lng,
      map.getBounds().getNorthWest().lat
    ].join(" "),
    [
      map.getBounds().getSouthWest().lng,
      map.getBounds().getSouthWest().lat
    ].join(" "),
    [
      map.getBounds().getSouthEast().lng,
      map.getBounds().getSouthEast().lat
    ].join(" "),
    [
      map.getBounds().getNorthEast().lng,
      map.getBounds().getNorthEast().lat
    ].join(" ")
    ].join(",")
    return bbox;
}

const getData = async (area, onGetData) => {
  if (area) {
    await API()["rawArea"](
      area,
      {
        onSuccess: (data) => {
          if (data && data.length > 0) {
            const features = data[0].jsonb_build_object;
            onGetData(features, area);
          }
        },
        onError: (error) => {
          console.log(error)
        }
      }
    );
  }
}

function OnLoadMap(props) {
  const map = useMap();
  
  useEffect(() => {
    props.onGetData(getBBoxString(map));
  }, []);
}

function MapEventHandlers(props) {
  const map = useMap();
  const [area, setArea] = useState(null);
  const zoom = map.getZoom();
  useMapEvent('moveend', () => {
    props.onGetData(area);
    setArea(getBBoxString(map));
  });
  useEffect(() => {
    if (zoom > 13) {
      props.onGetData(area);
    }
  }, [area, zoom]);
}

const grayOptions = { color: 'gray' }
const redOptions = { color: 'red' }

const PopupMarker = ({ data }) => {
  const map = useMap();
  const [refReady, setRefReady] = useState(false);
  let popupRef = useRef();

  useEffect(() => {
    if (refReady) {
      popupRef.openOn(map);
    }
  }, [refReady, map]);

  return (
    <Marker position={
      [data.geometry.coordinates[0][0][1],
      data.geometry.coordinates[0][0][0]]
    }>
      <Popup
        ref={(r) => {
          popupRef = r;
          setRefReady(true);
        }}
      >
        <h3>
          { data.properties.status == 'badgeom' ? "Un-squared building" : "Building"} 
        </h3>
        <p>
          <a target="_blank" href={"https://osm.org/way/" + data.properties.osm_id}>
            {data.properties.osm_id}
          </a>
        </p>
      </Popup>
    </Marker>
  );
};

// DQMap component
export const DQMap = ({
    className,
    center = [14.70884, -90.47560],
    realtime = true
  }) => {
    const [data, setData] = useState([]);
    const [area, setArea] = useState(null);
    const [selectedFeature, setSelectedFeature] = useState(null);
    const timeoutRef = useRef();

    const mapMoveHandler = (area) => {
      clearTimeout(timeoutRef.current);
      if (realtime) {
        timeoutRef.current = setInterval(() => {
          getData(area, (result) => {
            setData(result.features);
          });
        }, 5000);  
      }
      setArea(area);
    }

    const loadHandler = (area) => {
      setArea(area);
      if (realtime) {
        timeoutRef.current = setInterval(() => {
          getData(area, (result) => {
            setData(result.features);
          });
        }, 5000);  
      }
    }

    useEffect(() => {
        getData(area, (result) => {
          setData(result.features);
        });
    }, [area]);
    
    return <div className={className || "Map"}>
        <MapContainer 
          style={{height: 500}}
          center={center}
          zoom={17}
          scrollWheelZoom={false}
        >
        <TileLayer
          attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
          url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
          maxNativeZoom={19}
          maxZoom={25}
        />
        { data && data.filter(x => x.properties.status != 'badgeom').map(x =>
          <Polygon 
            key={x.properties.way_id}
            pathOptions={grayOptions}
            positions={x.geometry.coordinates[0].map(coord => 
              [coord[1], coord[0]]
            )
            } 
            clickable={true}
            eventHandlers={{
              click: (e) => {
                setSelectedFeature(x);
              }
            }}
          />
        )}
        { data && data.filter(x => x.properties.status == 'badgeom').map(x =>
          <Polygon 
            key={x.properties.way_id}
            pathOptions={redOptions}
            positions={x.geometry.coordinates[0].map(coord => 
              [coord[1], coord[0]]
            )
            } 
            clickable={true}
            eventHandlers={{
              click: (e) => {
                setSelectedFeature(x);
              }
            }}
          />
        )}
        <MapEventHandlers onGetData={mapMoveHandler} />
        { selectedFeature &&
          <PopupMarker data={selectedFeature} />
        }
        { area == null && 
          <OnLoadMap onGetData={loadHandler} 
          />
        }        
      </MapContainer>
    </div>
  }

