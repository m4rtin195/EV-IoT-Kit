const AWS = require('aws-sdk');
const ddb = new AWS.DynamoDB.DocumentClient();

const dbTableName = "fcm-registrations";
var lambda_reply = {headers: {'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*'}};

exports.handler = async function(event)
{
    if(!event.hasOwnProperty('body'))
    {
        console.error("'body' attribute was not found while parsing the request");
        lambda_reply.statusCode = 500;
        return lambda_reply;
    }
        
    let payload;
    try {payload = JSON.parse(event.body);} 
    catch(e) {console.error(e)}

    if(!(payload.vehicleId && payload.fcmToken && payload.hasOwnProperty('register')))
    {
        lambda_reply.statusCode = 400;
        return lambda_reply;
    }
    
    if(payload.register === true) //operation = register
    {
        await createDocument(payload.vehicleId, payload.fcmToken).then((reply) => 
        {
            if(Object.keys(reply).length === 0) //reply is empty
            {
                lambda_reply.statusCode = 200;
            }
            else
            {
                lambda_reply.statusCode = 500;
                console.error(JSON.stringify(reply))
            }
        }).catch((err) => {console.error(err)});;
    }
    
    if(payload.register === false) //operation = unregister
    {
        await deleteDocument(payload.vehicleId, payload.fcmToken).then((reply) => 
        {
            if(Object.keys(reply).length === 0) //reply is empty
            {
                lambda_reply.statusCode = 200;
            }
            else
            {
                lambda_reply.statusCode = 500;
                console.error(JSON.stringify(reply))
            }
        }).catch((err) => {console.error(err)});
    }
    return lambda_reply;
};

var createDocument = async function(vehicleId, fcmToken)
{
    let renewTimestamp = Math.floor(Date.now() / 1000);
    let document = {vehicleId, fcmToken, renewTimestamp};
    let query = 
    {
        TableName: dbTableName,
        Item: document
    };
    return ddb.put(query).promise();
}

var deleteDocument = async function(vehicleId, fcmToken)
{
    let document = {vehicleId, fcmToken};
    let query = 
    {
        TableName: dbTableName,
        Key: {'vehicleId': vehicleId, 'fcmToken': fcmToken}
    };
    return ddb.delete(query).promise();
}
