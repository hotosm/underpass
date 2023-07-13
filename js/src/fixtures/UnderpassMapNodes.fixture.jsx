import React from 'react';
import { UnderpassMapNodes } from '../components/Underpass/UnderpassMapNodes';
import { center } from './center';

const styles = {}

export default (
  <div>
    <div style={styles.report}>
      <UnderpassMapNodes
        center={center}
        tag="amenity"
        greyscaleMap
      />
    </div>
  </div>
)

