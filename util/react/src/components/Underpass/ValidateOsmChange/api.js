
var headers = new Headers();
headers.append("Content-Type", "application/json");

// Send request to the Underpass API
const API = (url) => {
    const API_URL = url || process.env.REACT_APP_UNDERPASS_API || "http://localhost:5480";
    return {
        dataQualityReview: async (osmchange, options) => {
            fetch(API_URL + "/osmchange/validate", {
                method: 'POST',
                headers: headers,
                body: JSON.stringify({
                    osmchange: osmchange,
                    check: "building"
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