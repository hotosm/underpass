import React from 'react';
import { UnderpassMap } from '../components/Underpass/UnderpassMap';
import { center } from './center';

const styles = {}

export default (
  <div>
    <div style={styles.report}>
      <UnderpassMap
        center={center}
        tag="building"
        highlightDataQualityIssues
        greyscaleMap
      />
    </div>
  </div>
)

