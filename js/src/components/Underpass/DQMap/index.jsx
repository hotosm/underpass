import React, { useState, useEffect } from 'react';
import { MapContainer, TileLayer, Marker, Popup, useMap } from 'react-leaflet'
import API from '../api';
import "./styles.css";
import "./leaflet.css";

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
    report = "geo"
  }) => {
    const [result, setResult] = useState(null);
    const [loading, setLoading] = useState(false);
    const [selectedIndex, setSelectedIndex] = useState(0);

    const reports = {
      "geo": "reportDataQualityGeo",
    };

    useEffect(() => {
      const getData = async () => {
        setLoading(true);
        await API(apiUrl)[reports[report]](
          fromDate,
          toDate,
          hashtags,
          {
            onSuccess: (data) => {
              setResult(data);
              setLoading(false);
              onSuccess && onSuccess(data);
            },
            onError: (error) => {
              setLoading(false);
              console.log(error)
            }
          }
        );
      }
      getData();
    }, [fromDate, toDate, hashtags]);
    if (!loading) {
      if (result && result.length > 0) {
        return <div>
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
            <Marker position={[x.lon, x.lat]}>
            <Popup>
              {x.link}
            </Popup>
          </Marker>
          )}
        </MapContainer>
        <button
          onClick={() => setSelectedIndex(selectedIndex < result.length - 1 ? selectedIndex + 1 : 0)}
        >
          Next ({selectedIndex + 1}/{result.length})
        </button>
      </div>
      }
      return "No results.";
    }
    return "Loading ...";
  }

