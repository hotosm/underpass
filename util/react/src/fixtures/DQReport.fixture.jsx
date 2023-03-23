import React from 'react';
import { DQReport } from '../components/Underpass/DQReport';

const styles = {
  report: {padding: 20, margin: 20, border: "1px solid #ddd"}
}

export default (
  <div>
    <div style={styles.report}>
      <DQReport
        fromDate={"2022-12-01T00:00:00"}
        toDate={"2022-12-30T23:59:59"}
        hashtags={["hotosm"]}
        report="tag"
      />
    </div>
    <div style={styles.report}>
      <DQReport
        fromDate={"2022-12-01T00:00:00"}
        toDate={"2022-12-30T23:59:59"}
        hashtags={[]}
        report="geo"
      />
    </div>
    <div style={styles.report}>
      <DQReport
        fromDate={"2022-12-01T00:00:00"}
        toDate={"2022-12-30T23:59:59"}
        hashtags={[]}
        report="tagStats"
      />
    </div>
  </div>
)

