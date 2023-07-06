import React from 'react';
import { DQMap } from '../components/Underpass/DQMap';
import { center } from './center';

const styles = {}

export default (
  <div>
    <div style={styles.report}>
      <DQMap
        center={center}
        realtime={true}
          />
    </div>
  </div>
)

