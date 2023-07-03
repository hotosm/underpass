import React from 'react';
import { DQMap } from '../components/Underpass/DQMap';

const styles = {}

export default (
  <div>
    <div style={styles.report}>
      <DQMap
        center={[-31.0646, -64.2981]}
        realtime={true}
          />
    </div>
  </div>
)

