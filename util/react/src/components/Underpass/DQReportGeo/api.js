
var headers = new Headers();
headers.append("Content-Type", "application/json");

// Send request to the Underpass API
const API = (url) => {
    const API_URL = url || process.env.REACT_APP_UNDERPASS_API || "http://localhost:5480/v1";
    return {
        dataQualityReview: async (fromDate, toDate, hashtags, options) => {
            fetch(API_URL + "/data-quality/report/geo", {
                method: 'GET',
                headers: headers,
                body: JSON.stringify({
                    fromDate: fromDate,
                    toDate: toDate,
                    hashtags: hashtags
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