const https = require('https');

// Toto je verzia pre Firebase FCM Legacy HTTP API

module.exports = 
{
    sendMessage: function(message, target)
    {
        var promise = new Promise((resolve, reject) => 
        {
           const options = 
           {
               host: "fcm.googleapis.com",
               path: "/fcm/send",
               method: "POST",
               headers:
               {
                   "Authorization" : process.env.FCM_APIkey,
                   "Content-Type" : "application/json",
               }
           };
           
           const request = https.request(options, (result) =>
           {
                //console.log('success odoslane na fcm');
                //console.log(result.statusCode);
                resolve('success');
           });
           
           request.on('error', (e) =>
           {
                console.error('failure: ' + e.message);
                reject(e.message);
           });
           
           const reqBody = 
           {
               "to": target,
               "data": message
           }

           request.write(JSON.stringify(reqBody));
           request.end();
        });
        
        return promise;
    }
};