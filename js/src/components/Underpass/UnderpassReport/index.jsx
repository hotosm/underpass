import React, { useState, useEffect } from 'react';
import API from '../api';
import "./styles.css";

// UnderpassReport component
export const UnderpassReport = ({
    fromDate,
    toDate,
    hashtags,
    page,
    onSuccess,
    apiUrl,
    className,
    report = "tag"
  }) => {
    const [result, setResult] = useState(null);
    const [loading, setLoading] = useState(false);

    const reports = {
      "tag": "reportDataQualityTag",
      "geo": "reportDataQualityGeo",
      "tagStats": "reportDataQualityTagStats",
    };

    useEffect(() => {
      const getData = async () => {
        setLoading(true);
        await API(apiUrl)[reports[report]](
          fromDate,
          toDate,
          hashtags,
          page,
          {
            onSuccess: (data) => {
              setResult(data);
              setLoading(false);
              onSuccess && onSuccess(data);
            },
            onError: (error) => {
              setLoading(false);
              console.log(error)
            }
          }
        );
      }
      getData();
    }, [fromDate, toDate, hashtags]);

    if (!loading) {
      if (result && result.length > 0) {
        return <table className={className || "ReportResultTable"}>
          <thead>
            <tr>
              {
                Object.keys(result[0]).map(key => <th key={key}>{key}</th> )
              }
            </tr>
          </thead>
          <tbody>
            {
            result.map((row, index) => (
                <tr key={index}>
                  {
                    Object.keys(row).map((key) => {
                      if (row[key].toString().indexOf("http") > -1) {
                        return <td key={key}>
                          <a href={row[key]}>{row[key]}</a>
                        </td>
                      } else {
                        return <td key={key}>{row[key]}</td>
                      }
                    })
                  }
                </tr>
            ))
          }
        </tbody>
        </table>
      }
      return "No results.";
    }
    return "Loading ...";
  }

