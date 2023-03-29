import React from 'react';
import { DQReportDownload } from '../components/Underpass/DQReportDownload';

const fromDate = () => {
  const now = new Date();
  const d = new Date(now.getTime() - 7 * 24 * 60 * 60 * 1000);
  const datestring = d.getFullYear()  + "-" + (d.getMonth()+1) + "-" + d.getDate() + "T" +
  d.getHours() + ":" + d.getMinutes() + ":00"; 
  return datestring; 
}

export default (
    <div>
      <DQReportDownload
        fromDate={fromDate()}
        hashtags={[]}
        report="tag"
        text="Download Tag Report"
      />
      <br /><br />
      <DQReportDownload 
        fromDate={fromDate()}
        hashtags={[]}
        report="geo"
        text="Download Geo Report"
      />
      <br /><br />
      <DQReportDownload
        fromDate={fromDate()}
        hashtags={[]}
        report="tagStats"
        text="Download Tag Stats Report"
      />
    </div>
    )

