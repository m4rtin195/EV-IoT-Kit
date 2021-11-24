const https = require('https');

module.exports = 
{
    sendMessage: function(message, targets)
    {
        console.log("som v sendMessage in fcm.js");
        
        var promise = new Promise((resolve, reject) => 
        {
           const options = 
           {
               host: "fcm.googleapis.com",
               path: "/fcm/send",
               method: "POST",
               headers:
               {
                   "Authorization" : process.env.FCM_authorization,
                   "Content-Type" : "application/json",
               }
           };
           
           const request = https.request(options, (result) =>
           {
                console.log('success odoslane na fcm');
                console.log(result.statusCode);
                resolve('success');
           });
           
           request.on('error', (e) =>
           {
                console.log('failure' + e.message);
                reject(e.message);
           });
           
           const reqBody = 
           {
               "to" : "fn3ShLZ7S4ul2hbFEmo2Hg:APA91bG-Gdh0VH7hWtipd4lNwqRT6DhjloXaB7FLnXqN1YQcSI-BNKTi3Cw0KrJWAfNV-sWAPKStAe4LuOpN8zHnmYajQ2yuXKLrI5uDSZBgnHTpRBGL5fqg3hoh_FgjBiZB8iJKVKpR",
               "data" : message
           }
           console.log(options);
           console.log(reqBody);
           
           request.write(JSON.stringify(reqBody));
           request.end();
        });
        
        return promise;
    }
};


//        MessageStructure: 'json'
