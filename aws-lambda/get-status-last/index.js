const AWS = require('aws-sdk');
const ddb = new AWS.DynamoDB.DocumentClient();

const dbTableName = "vehicle-statuses";
var lambda_reply = {headers: {'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*'}};

exports.handler = async function(event, context, callback)
{
    if(!event.hasOwnProperty('queryStringParameters'))
    {
        console.error("'queryStringParameters' attribute was not found while parsing the request");
        lambda_reply.statusCode = 500;
        callback(null, lambda_reply);
    }
    
    let qsp = event.queryStringParameters;
    if(!qsp.vehicleId)
    {
        console.error("Missing basic attributes");
        lambda_reply.body = JSON.stringify({"Error": "Missing basic attributes"}, null, 2);
        lambda_reply.statusCode = 400;
        callback(null, lambda_reply);
        return;
    }
    
    let vehicleId = qsp.vehicleId;
    //let vehicleId = "386625";
    
    //read from DynamoDB
    await readDocument(vehicleId).then(data =>
    {
        if(data.Items[0])
        {
            lambda_reply.body = JSON.stringify(data.Items[0], null, 2);
            lambda_reply.statusCode = 200;
            callback(null, lambda_reply);
        }
        else
        {
            lambda_reply.statusCode = 204;
            callback(null, lambda_reply);
        }
    }).catch((err) => console.error(err));
};

function readDocument(vehicleId)
{
    const request = 
    {
        TableName: 'vehicle-statuses',
        IndexName: 'vehicleId-timestamp-index',
        KeyConditionExpression: 'vehicleId = :vehicleId',
        ExpressionAttributeValues: {":vehicleId": vehicleId},
        ScanIndexForward: false, //sort order
        Limit: 1
    }
    
    return ddb.query(request).promise();
}
