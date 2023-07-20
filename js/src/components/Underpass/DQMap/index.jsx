import React, { useEffect, useState, useRef } from 'react';
import { createRoot } from 'react-dom/client';
import maplibregl from 'maplibre-gl';
import 'maplibre-gl/dist/maplibre-gl.css';

import hottheme from '../../HOTTheme';
import API from '../api';
import { mapStyle } from './mapStyle';
import './styles.css';

const MIN_ZOOM = 13;

export default function UnderpassMap({ center, theme: propTheme = {} }) {
  const mapContainer = useRef(null);
  const mapRef = useRef(null);
  const [map, setMap] = useState(null);
  const popUpRef = useRef(
    new maplibregl.Popup({ closeOnMove: true, closeButton: false })
  );

  const theme = {
    colors: {
      ...hottheme.colors,
      ...propTheme.colors,
    },
    polygon: {
      ...hottheme.polygon,
      ...propTheme.polygon,
    },
  };

  const themeFromProp = {
    '--primary': theme.colors.primary,
  };

  useEffect(() => {
    if (mapRef.current) return;

    mapRef.current = new maplibregl.Map({
      container: mapContainer.current,
      style: mapStyle,
      center: center.reverse(),
      zoom: 17,
    });
    mapRef.current.addControl(new maplibregl.NavigationControl());
    setMap(mapRef.current);
  }, [center]);

  useEffect(() => {
    if (!map) return;
    async function fetchBuildings() {
      await API()['rawPolygons'](getBBoxString(map), 'building', {
        onSuccess: (data) => {
          if (map.getSource('buildings')) {
            map.getSource('buildings').setData(data);
          } else {
            map.addSource('buildings', {
              type: 'geojson',
              data: data,
            });
            map.addLayer({
              id: 'buildings',
              source: 'buildings',
              type: 'fill',
              layout: {},
              paint: {
                'fill-color': [
                  'match',
                  ['get', 'status'],
                  'badgeom',
                  `rgb(${theme.colors.primary})`,
                  `rgb(${theme.colors.secondary})`,
                ],
                'fill-opacity': theme.polygon.fillOpacity,
              },
            });
            map.addLayer({
              id: 'outline',
              type: 'line',
              source: 'buildings',
              layout: {},
              paint: {
                'line-color': [
                  'match',
                  ['get', 'status'],
                  'badgeom',
                  `rgb(${theme.colors.primary})`,
                  `rgb(${theme.colors.secondary})`,
                ],
                'line-width': theme.polygon.outlineWidth,
              },
            });
          }
        },
        onError: (error) => {
          console.log(error);
        },
      });
    }

    map.on('load', () => {
      fetchBuildings();
    });

    map.on('moveend', () => {
      const zoom = map.getZoom();
      zoom > MIN_ZOOM && fetchBuildings();
    });

    map.on('click', 'buildings', (e) => {
      const popupNode = document.createElement('div');
      createRoot(popupNode).render(<Popup feature={e.features[0]} />);
      popUpRef.current.setLngLat(e.lngLat).setDOMContent(popupNode).addTo(map);
    });

    // Display pointer cursor for polygons
    map.on('mouseenter', 'buildings', () => {
      map.getCanvas().style.cursor = 'pointer';
    });

    map.on('mouseleave', 'buildings', () => {
      map.getCanvas().style.cursor = '';
    });
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [map]);

  return (
    <div style={themeFromProp} className='map-wrap'>
      <div ref={mapContainer} className='map' />
    </div>
  );
}

const Popup = ({ feature }) => {
  const tags = JSON.parse(feature.properties.tags);

  return (
    <div className='popup'>
      <table>
        <tbody>
          <tr>
            <td colSpan='2'>
              <b>Way:</b>&nbsp;
              <a
                target='blank'
                href={`https://www.openstreetmap.org/way/${feature.id}`}
              >
                {feature.id}
              </a>
            </td>
          </tr>
          {Object.keys(tags).map((tag) => (
            <tr key={tag}>
              <td>{tag}</td>
              <td>{tags[tag]}</td>
            </tr>
          ))}
          {feature.properties.status && (
            <tr>
              <td colSpan='2'>
                <strong className='status'>{feature.properties.status}</strong>
              </td>
            </tr>
          )}
        </tbody>
      </table>
    </div>
  );
};

function getBBoxString(map) {
  let bbox = [
    [
      map.getBounds().getNorthEast().lng,
      map.getBounds().getNorthEast().lat,
    ].join(' '),
    [
      map.getBounds().getNorthWest().lng,
      map.getBounds().getNorthWest().lat,
    ].join(' '),
    [
      map.getBounds().getSouthWest().lng,
      map.getBounds().getSouthWest().lat,
    ].join(' '),
    [
      map.getBounds().getSouthEast().lng,
      map.getBounds().getSouthEast().lat,
    ].join(' '),
    [
      map.getBounds().getNorthEast().lng,
      map.getBounds().getNorthEast().lat,
    ].join(' '),
  ].join(',');
  return bbox;
}
