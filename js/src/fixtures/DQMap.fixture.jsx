import React from 'react';
import { DQMap } from '../components/Underpass/DQMap';

const styles = {}

export default (
  <div>
    <div style={styles.report}>
      <DQMap
        center={[28.20784, 83.99706]}
        realtime={true}
          />
    </div>
  </div>
)

