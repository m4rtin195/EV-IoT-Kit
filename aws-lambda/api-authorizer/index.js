const AWS = require('aws-sdk');
const ddb = new AWS.DynamoDB.DocumentClient();

const dbTableName = "vehicle-registrations";

exports.handler = async function(event)
{
    //causes case-sensitive errors but http headers are case insensitive, and this case anyway cannot happen because api gateway check requirements
    /*if(!(event.headers.hasOwnProperty('userId') && event.queryStringParameters.hasOwnProperty('vehicleId')))
        throw Error(400);*/ 
    
    const keyname = Object.keys(event.headers).find(key => key.toLowerCase() === 'userId'.toLowerCase());
    
    let userId = event.headers[keyname];
    let vehicleId = event.queryStringParameters.vehicleId;
    
    let policy;
    let granted = await checkOwnership(userId, vehicleId);
    
    if(granted)
        policy = generatePolicy('Allow', event.methodArn);
    else
        policy = generatePolicy('Deny', event.methodArn);
    
    return policy;
};

var checkOwnership = async function(userId, vehicleId)
{
    //let query = 'SELECT * FROM "' + dbTableName + '" WHERE userId="' + userId + '" AND vehicleId="' + vehicleId + '"';  //for PartiQL
    let query = 
    {
        TableName: dbTableName,
        KeyConditionExpression: 'userId = :userId AND vehicleId = :vehicleId',
        ExpressionAttributeValues: {":userId" : userId, ":vehicleId" : vehicleId}
    }

    let response = await ddb.query(query).promise();
    if(response.Count > 0)
        return true;
    else
        return false;
}

// Help function to generate an IAM policy
var generatePolicy = function(effect, resource)
{
    var authResponse = {};
    authResponse.principalId = '*';

    if(effect && resource)
    {
        var policyDocument = {};
        policyDocument.Version = '2012-10-17'; 
        policyDocument.Statement = [];
        var statementOne = {};
        statementOne.Action = 'execute-api:Invoke'; 
        statementOne.Effect = effect;
        statementOne.Resource = resource;
        policyDocument.Statement[0] = statementOne;
        authResponse.policyDocument = policyDocument;
    }
    
    return authResponse;
}