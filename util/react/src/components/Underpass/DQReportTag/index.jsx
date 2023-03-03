import React, { useState, useEffect } from 'react';
import API from './api';
import "./styles.css";

// DQReportTag component
export const DQReportTag = ({
    fromDate,
    toDate,
    hashtags,
    responseType,
    onSuccess,
    apiUrl,
    className
  }) => {
    const [report, setReport] = useState(null);
    
    useEffect(() => {
      const getData = async () => {
        await API(apiUrl).reportDataQualityTag(
          fromDate,
          toDate,
          hashtags,
          responseType,
          {
            onSuccess: (result) => {
              setReport(result);
              onSuccess && onSuccess(result);
            },
            onError: (error) => console.log(error)
          }
        );
      }
      getData();
    }, [fromDate, toDate, hashtags]);

    if (report) {
        return <table className={className || "ReportResultTable"}>
          <tr>
            {
              Object.keys(report[0]).map((key, index) => <th>{key}</th> )
            }
          </tr>
          {
          report.map(row => (
            <tr>
              {
                Object.keys(row).map((key, index) => {
                  if (row[key].indexOf("http") > -1) {
                    return <td>
                      <a href={row[key]}>{row[key]}</a>
                    </td>
                  } else {
                    return <td>{row[key]}</td>
                  }
                })
              }
            </tr>
          ))
        }
        </table>
    }

    return "Loading ...";
  }

