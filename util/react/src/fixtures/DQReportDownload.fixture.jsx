import React from 'react';
import { DQReportDownload } from '../components/Underpass/DQReportDownload';

export default (
    <div>
      <DQReportDownload
        fromDate={"2022-12-01T00:00:00"}
        hashtags={[]}
        report="tag"
        text="Download Tag Report"
      />
      <br /><br />
      <DQReportDownload 
        fromDate={"2022-12-01T00:00:00"}
        hashtags={[]}
        report="geo"
        text="Download Geo Report"
      />
      <br /><br />
      <DQReportDownload
        fromDate={"2022-12-01T00:00:00"}
        hashtags={[]}
        report="tagStats"
        text="Download Tag Stats Report"
      />
    </div>
    )

