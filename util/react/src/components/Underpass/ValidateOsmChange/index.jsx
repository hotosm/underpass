import React, { useState, useEffect } from 'react';
import JXON from './jxon';
import API from './api';

// ValidateOsmChange component
export const ValidateOsmChange = ({ osmchange, onReview, apiUrl }) => {
    const [review, setReview] = useState(null);
    
    useEffect(() => {
      const getData = async () => {
        if (typeof osmchange !== 'string') {
          osmchange = JXON.stringify(osmchange);
        }
        await API(apiUrl).dataQualityReview(osmchange, {
          onSuccess: (result) => {
            setReview(result);
            onReview && onReview(result);
          },
          onError: (error) => console.log(error)
        });
      }
      getData();
    }, [osmchange]);

    if (review) {
      const issues = review.filter(issue => issue.results.indexOf("badvalue") > -1);
      if (issues.length) {
        const badValues =  issues.map( issue => {
          return "Bad value: " + issue.values.join(",");
        });
        return (
          <div>
            <h2>We've found {issues.length} issue {issues.length > 1 && "s"}</h2>
            { badValues }
          </div>
        );
      }
    }
    return null;
  }

