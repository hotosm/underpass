
var headers = new Headers();
headers.append("Content-Type", "application/json");

// Send request to the Underpass API
const API = (url) => {
    const API_URL = url ||
        process.env.REACT_APP_UNDERPASS_API
        || "http://localhost:8000";
    return {
        reportDataQualityTag: async (fromDate, toDate, hashtags, options = {}) => {
            fetch(API_URL + "/report/dataQualityTag", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    fromDate: fromDate,
                    toDate: toDate,
                    hashtags: hashtags
                })
            })
            .then(res => {
                return res.json();
            })
            .then(
                (result) => {
                    options.onSuccess && options.onSuccess(result);
                },
                (error) => {
                    options.onError && options.onError(error);
                }
            )
        },

        reportDataQualityTagCSV: async (fromDate, toDate, hashtags = [], options = {}) => {
            fetch(API_URL + "/report/dataQualityTag/csv", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    fromDate: fromDate,
                    toDate: toDate,
                    hashtags: hashtags
                })
            })
            .then(res => {
                return res.text();
            })
            .then(
                (result) => {
                    options.onSuccess && options.onSuccess(result);
                },
                (error) => {
                    options.onError && options.onError(error);
                }
            )
        },

        reportDataQualityGeo: async (fromDate, toDate, hashtags = [], options = {}) => {
            fetch(API_URL + "/report/dataQualityGeo", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    fromDate: fromDate,
                    toDate: toDate,
                    hashtags: hashtags
                })
            })
            .then(res => {
                return res.json();
            })
            .then(
                (result) => {
                    options.onSuccess && options.onSuccess(result);
                },
                (error) => {
                    options.onError && options.onError(error);
                }
            )
        },

        reportDataQualityGeoCSV: async (fromDate, toDate, hashtags = [], options = {}) => {
            fetch(API_URL + "/report/dataQualityGeo/csv", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    fromDate: fromDate,
                    toDate: toDate,
                    hashtags: hashtags
                })
            })
            .then(res => {
                return res.text();
            })
            .then(
                (result) => {
                    options.onSuccess && options.onSuccess(result);
                },
                (error) => {
                    options.onError && options.onError(error);
                }
            )
        },

        reportDataQualityTagStats: async (fromDate, toDate, hashtags = [], options = {}) => {
            fetch(API_URL + "/report/dataQualityTagStats", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    fromDate: fromDate,
                    toDate: toDate,
                    hashtags: hashtags
                })
            })
            .then(res => {
                return res.json();
            })
            .then(
                (result) => {
                    options.onSuccess && options.onSuccess(result);
                },
                (error) => {
                    options.onError && options.onError(error);
                }
            )
        },

        reportDataQualityTagStatsCSV: async (fromDate, toDate, hashtags = [], options = {}) => {
            fetch(API_URL + "/report/dataQualityTagStats/csv", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    fromDate: fromDate,
                    toDate: toDate,
                    hashtags: hashtags
                })
            })
            .then(res => {
                return res.text();
            })
            .then(
                (result) => {
                    options.onSuccess && options.onSuccess(result);
                },
                (error) => {
                    options.onError && options.onError(error);
                }
            )
        },

        dataQualityReview: async (osmchange, check = "building", options = {}) => {
            fetch(API_URL + "/osmchange/validate", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    osmchange: osmchange,
                    check: check
                })
            })
            .then(res => res.json())
            .then(
                (result) => {
                    options.onSuccess && options.onSuccess(result);
                },
                (error) => {
                    options.onError && options.onError(error);
                }
            )
        }
    }
}

export default API;