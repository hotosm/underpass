import React, { useState, useEffect, useRef } from 'react';
import { MapContainer, TileLayer, Marker, Popup, useMap } from 'react-leaflet'
import API from '../api';
import "./styles.css";
import 'leaflet/dist/leaflet.css';

function ChangeView({ center, zoom }) {
  const map = useMap();
  map.setView(center, zoom);
  return null;
}

// DQMap component
export const DQMap = ({
    fromDate,
    toDate,
    hashtags,
    onSuccess,
    apiUrl,
    className,
    report = "geo",
    page = 0
  }) => {
    const [result, setResult] = useState(null);
    const [loading, setLoading] = useState(false);
    const [selectedIndex, setSelectedIndex] = useState(0);
    const popup = useRef();

    const reports = {
      "geo": "reportDataQualityGeo",
    };

    function closePopup() {
      popup.current.close();
    }

    useEffect(() => {
      const getData = async () => {
        setLoading(true);
        await API(apiUrl)[reports[report]](
          fromDate,
          toDate,
          hashtags,
          page,
          {
            onSuccess: (data) => {
              setResult(data);
              setLoading(false);
              onSuccess && onSuccess(data);
            },
            onError: (error) => {
              console.log(error)
              setLoading(false);
            }
          }
        );
      }
      getData();
    }, [fromDate, toDate, hashtags]);

    if (!loading) {
      if (result && result.length > 0) {
        return <div className={className || "Map"}>
          <MapContainer 
            style={{height: 500}}
            center={[result[0].lon, result[0].lat]}
            zoom={18}
            scrollWheelZoom={false}
          >
          <ChangeView center={[result[selectedIndex].lon, result[selectedIndex].lat]} zoom={18} /> 
          <TileLayer
            attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
            url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
          />
          {result.map(x => 
            <Marker
              position={[x.lon, x.lat]}
            >
            <Popup ref={popup}>
              <a href={x.link} target="_blank">{x.link}</a>
            </Popup>
          </Marker>
          )}
        </MapContainer>
        <button
          className={"button"}
          onClick={() => {
            setSelectedIndex(selectedIndex < result.length - 1 ? selectedIndex + 1 : 0);
            closePopup();
          }}
        >
          Next ({selectedIndex + 1}/{result.length})
        </button>
      </div>
      }
      return "No results.";
    }
    return "Loading ...";
  }

