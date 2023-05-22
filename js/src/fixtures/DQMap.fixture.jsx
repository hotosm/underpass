import React from 'react';
import { DQMap } from '../components/Underpass/DQMap';

const styles = {}

export default (
  <div>
    <div style={styles.report}>
      <DQMap
        center={[14.70884, -90.47560]}
        realtime={true}
          />
    </div>
  </div>
)

