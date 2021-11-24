const https = require('https');
const {auth} = require('google-auth-library');

const firebaseProjectId = '681435988369';

module.exports =
{
    sendMessage: async function(payload, targets)
    {
        let api_access_token = await getAccessToken();
        
        var promise = new Promise((resolve, reject) => 
        {
            let httpRequestsPromises = [];
            let withoutFail = true;
            let nFailed = 0;
            
            //http request options
            const options =
            {
                host: 'fcm.googleapis.com',
                path: `/v1/projects/${firebaseProjectId}/messages:send`,
                method: 'POST',
                headers: 
                {
                    'Content-Type': 'application/json',
                    'Authorization': 'Bearer ' + api_access_token
                }
            };
            
            targets.forEach(target =>
            {
                //http request body
                const body = 
                {
                    message: 
                    {
                        token: target,
                        data: {"serialized": JSON.stringify(payload)}, //payload
                        android: {
                            ttl: '60s',
                            priority: 'NORMAL',
                            restricted_package_name: 'com.martin.carcharge',
                        }
                    }
                };
                
                httpRequestsPromises.push(new Promise((resolveNested, rejectNested) => 
                {
                    //http request
                    const request = https.request(options, (result) =>
                    {
                        result.on('data', (d) =>    //HTTP request successful
                        {
                            //console.log(`statusCode: ${result.statusCode}`);
                            //process.stdout.write(d); //write reply to console
                            if(result.statusCode !== 200)
                            {
                                withoutFail = false;
                                nFailed++;
                            }
                            resolveNested();
                        })
                    });
                    
                    request.on('error', (error) =>  //HTTP request failed
                    {
                        console.error(error);
                        rejectNested(error);
                    });
                
                    request.write(JSON.stringify(body));
                    request.end();
                })); //httpRequestsPromises push
            }); //forEach target
            
            
            //final evaluation after all http requests have finished
            Promise.all(httpRequestsPromises).then(() => 
            {
                //console.log("withoutFail: " + withoutFail);
                if(withoutFail === true) //all tokens success
                    resolve(200);
                else                //some tokens failed
                    resolve(`200: But ${nFailed} of ${targets.length} tokens failed.`);
            }).catch(error => console.error(error.message));
        });
        
        return promise;
    }
}


// Mint OAuth 2.0 access token using Google Auth Library
async function getAccessToken()
{
    //load the environment variable with service account keys
    const keysEnvVar = process.env.firebase_account_keys;
    if(!keysEnvVar)
      throw new Error('The $firebase_account_keys environment variable was not found!');
    const keys = JSON.parse(keysEnvVar);

    //load the JWT /*or UserRefreshClient (?)*/ from the keys
    const client = auth.fromJSON(keys);
    client.scopes = ['https://www.googleapis.com/auth/firebase.messaging'];
    
    //request token creation
    let access_token;
    await client.authorize().then((credentials) => access_token = credentials.access_token);
    
    return access_token;
}