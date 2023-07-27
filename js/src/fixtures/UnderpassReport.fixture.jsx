import React from 'react';
import { UnderpassReport } from '../components/Underpass/UnderpassReport';

const fromDate = () => {
  const now = new Date();
  const d = new Date(now.getTime() - 1 * 24 * 60 * 60 * 1000);
  const datestring = d.getFullYear()  + "-" + (d.getMonth()+1) + "-" + d.getDate() + "T" +
  d.getHours() + ":" + d.getMinutes() + ":00"; 
  return datestring; 
}


const styles = {
  report: {padding: 20, margin: 20, border: "1px solid #ddd"}
}

export default (
  <div>
    <div style={styles.report}>
      <UnderpassReport
        fromDate={fromDate()}
        hashtags={[]}
        report="geo"
      />
    </div>
    {/* <div style={styles.report}>
      <UnderpassReport
        fromDate={fromDate()}
        hashtags={[]}
        report="tag"
      />
    </div>
    <div style={styles.report}>
      <UnderpassReport
        fromDate={fromDate()}
        hashtags={[]}
        report="tagStats"
      />
    </div> */}
  </div>
)

