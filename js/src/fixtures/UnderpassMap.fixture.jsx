import React from 'react';
import UnderpassMap from '../components/Underpass/UnderpassMap';
import { center } from './center';
import hottheme from '../components/HOTTheme';
// import './UnderpassMap.css';

const HOTTheme = hottheme();

const theme = {
  // map: {
  //   ...HOTTheme.map,
  //   waysLine: {
  //     ...HOTTheme.map.waysLine,
  //     'line-color': [
  //       'match',
  //       ['get', 'status'],
  //       'badgeom',
  //       `yellow`,
  //       `green`
  //     ],
  //     'line-width': 3,
  //   },
  //   raster: {
  //     'raster-opacity': 1,
  //     'raster-brightness-max': 1,
  //     'raster-saturation': 0,
  //   }
  // }
};

export default <UnderpassMap 
  mapClassName={"customMap"}
  center={center}
  theme={theme}
  isRealTime
  tag="building"
/>;
