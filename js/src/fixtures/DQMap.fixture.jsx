import React from 'react';
import { DQMap } from '../components/Underpass/DQMap';

const fromDate = () => {
  const now = new Date();
  const d = new Date(now.getTime() - 1 * 24 * 60 * 60 * 1000);
  const datestring = d.getFullYear()  + "-" + (d.getMonth()+1) + "-" + d.getDate() + "T" +
  d.getHours() + ":" + d.getMinutes() + ":00"; 
  return datestring; 
}

const styles = {}

export default (
  <div>
    <div style={styles.report}>
      <DQMap
        report="geo"
      />
    </div>
  </div>
)

