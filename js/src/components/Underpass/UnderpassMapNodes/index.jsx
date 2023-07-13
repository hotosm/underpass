import React, { useState, useEffect, useRef } from 'react';
import { MapContainer, TileLayer, useMap, useMapEvent, Circle, Popup, Marker } from 'react-leaflet'
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

const getData = async (area, tag, onGetData) => {
  if (area) {
    await API()["rawNodes"](
      area,
      tag,
      {
        onSuccess: (data) => {
          if (data) {
            const features = data;
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
      [data.geometry.coordinates[1],
      data.geometry.coordinates[0]]
    }>
      <Popup
        ref={(r) => {
          popupRef = r;
          setRefReady(true);
        }}
      >
        <p>
        Node: <a target="_blank" href={"https://osm.org/node/" + data.id}>
             {data.id}
          </a>
        </p>
        <table>
          { Object.keys(data.properties.tags).map(tag => <tr>
            <td><strong>{tag}</strong>:</td><td>{data.properties.tags[tag]}</td>
            </tr>) } 
        </table>
      </Popup>
    </Marker>
  );
};

// UnderpassMapNodes component
export const UnderpassMapNodes = ({
    className,
    center = [14.70884, -90.47560],
    tag,
    realtime = false,
    nodeStyles = {
      color: 'rgba(71,159,248)',
      fillColor: 'rgba(71,159,248)',
      fillOpacity: .2,
      weight: 1.5
    },
    greyscaleMap = true
  }) => {
    const [data, setData] = useState([]);
    const [area, setArea] = useState(null);
    const [selectedFeature, setSelectedFeature] = useState(null);
    const timeoutRef = useRef();

    const mapMoveHandler = (area) => {
      clearTimeout(timeoutRef.current);
      if (realtime) {
        timeoutRef.current = setInterval(() => {
          getData(area, tag, (result) => {
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
          getData(area, tag, (result) => {
            setData(result.features);
          });
        }, 5000);
      }
    }

    useEffect(() => {
        getData(area, tag, (result) => {
          setData(result.features);
        });
    }, [area]);
    
    return <div className={className || greyscaleMap ? "MapGrayscale" : "Map"}>
        <MapContainer 
          style={{height: 500}}
          center={center}
          zoom={19}
          scrollWheelZoom={false}
        >
        <TileLayer
          attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
          url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
          maxNativeZoom={19}
          maxZoom={25}
        />
          { data && data.map(item =>
            <Circle
              key={item.properties.way_id}
              center={{lat: item.geometry.coordinates[1], lng: item.geometry.coordinates[0]}}
              radius={3}
              clickable={true}
              eventHandlers={{
                click: (e) => {
                  setSelectedFeature(item);
                }
              }}
              {...nodeStyles}
            />
          )
        }
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

