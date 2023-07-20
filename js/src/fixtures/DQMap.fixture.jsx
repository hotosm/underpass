import React from 'react';

import DQMap from '../components/Underpass/DQMap';
import { center } from './center';

const theme = {
  polygon: {
    outlineWidth: 1.5,
  },
};

export default <DQMap center={center} theme={theme} isRealTime />;
