import React, { useState, useEffect } from 'react';
import API from '../api';
import "./styles.css";

// DQReportDownload component
export const DQReportDownload = ({
    fromDate,
    toDate,
    hashtags,
    onSuccess,
    apiUrl,
    className,
    report = "tag",
    text
  }) => {
    const [result, setResult] = useState(null);
    const [loading, setLoading] = useState(false);
    const reports = {
      "tag": "reportDataQualityTagCSV",
      "geo": "reportDataQualityGeoCSV",
      "tagStats": "reportDataQualityTagStatsCSV",
    };

    const getData = async () => {
      setLoading(true);
      await API(apiUrl)[reports[report]] (
        fromDate,
        toDate,
        hashtags,
        {
          onSuccess: (data) => {
            setResult(data);
            setLoading(false);
            onSuccess && onSuccess(data);
          },
          onError: (error) => {
            setLoading(false);
            console.log(error);
          }
        }
      );
    }

    useEffect(() => {
      const download = (data) => {
        const blob = new Blob([data], {
          type: "application/octet-stream",
          endings: "native"
        });
        const url = URL.createObjectURL(blob);
        var element = document.createElement('a');
        element.setAttribute('href', url);
        element.setAttribute('download', [
          report,fromDate,toDate
        ].concat(hashtags).join("-") + ".csv");
        element.style.display = 'none';
        document.body.appendChild(element);
        element.click();
        document.body.removeChild(element);
      }
      if (result) {
        download(result);
      }
    }, [result]);

    return (
        <button
          className={className || "ReportDownloadLink"}
          onClick={() => getData()}
        >
          { loading ? "Loading..." : text || "Download" }
        </button>
    )
  }

