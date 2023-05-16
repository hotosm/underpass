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

function MapEventHandlers(props) {
  const map = useMap();
  const [area, setArea] = useState(null);
  useMapEvent('moveend', () => {
    setArea(getBBoxString(map));
  });
  useEffect(() => {
    const getData = async () => {
      await API()["rawArea"](
        area,
        {
          onSuccess: (data) => {
            if (data.length > 0) {
              const features = data[0].jsonb_build_object;
              props.onGetData(features);
            }
          },
          onError: (error) => {
            console.log(error)
          }
        }
      );
    }
    getData();
  }, [area]);

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
        <table>
          <tbody>
            <tr>
              <td>
                <a target="_blank" href={"https://osm.org/way/" + data.properties.way_id}>
                 {data.properties.way_id}
                </a>
              </td>
            </tr>
          </tbody>
        </table>
      </Popup>
    </Marker>
  );
};

// DQMap component
export const DQMap = ({
    className
  }) => {
    const [data, setData] = useState([]);
    const [selectedFeature, setSelectedFeature] = useState(null);
    const dataHandler = (result) => {
      setData(result.features);
    }
    return <div className={className || "Map"}>
        <MapContainer 
          style={{height: 500}}
          center={[14.99184, -90.811215]}
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
        <MapEventHandlers onGetData={dataHandler} />
        { selectedFeature &&
          <PopupMarker data={selectedFeature} />
        }
      </MapContainer>
    </div>
  }

