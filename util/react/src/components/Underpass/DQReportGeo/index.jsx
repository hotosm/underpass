import React, { useState, useEffect } from 'react';
import API from './api';

// DQReportGeo component
export const DQReportGeo = ({ fromDate, toDate, hashtags, onSuccess, apiUrl }) => {
    const [report, setReport] = useState(null);
    
    useEffect(() => {
      const getData = async () => {
        await API(apiUrl).dataQualityReview(fromDate, toDate, hashtags, {
          onSuccess: (result) => {
            setReport(result);
            onSuccess && onSuccess(result);
          },
          onError: (error) => console.log(error)
        });
      }
      getData();
    }, [fromDate, toDate, hashtags]);

    if (report) {
        <code>{report}</code>
    }

    return null;
  }

