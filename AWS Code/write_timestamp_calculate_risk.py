# This file is for an AWS Lambda function of the same name
# This Lambda function automatically appends a createdDate property and
# computes roll and pitch data from an incoming AWS IoT MQTT update message
# and writes it into DynamoDB.
# Its secondary function is to recompute the risk factor whenever a new
# entry has been added to the database

import datetime
import time
import math
import decimal
import boto3

dynamodb = boto3.resource('dynamodb')
entries_table = dynamodb.Table('entries')
devices_table = dynamodb.Table('devices')

def lambda_handler(event, context):
    # For debugging purposes, print out event
    print("DEBUG: Received event with data: ")
    print(event)

    # Get current UNIX timestamp
    dts = datetime.datetime.utcnow()
    epochtime = round(time.mktime(dts.timetuple()) + dts.microsecond/1e6)
    print("DEBUG: Current UNIX timestamp: ")
    print(epochtime)

    # Calculate roll and pitch (in degrees) for that sensor value in
    # Formula available here: https://www.nxp.com/docs/en/application-note/AN3461.pdf
    # Roll range: [0, 180], pitch range: [0, 90]
    x = event['accl_x']
    y = event['accl_z'] # z and y are swapped as the sensor is mounted
    z = event['accl_y'] # vertically, not horizontally
    rain = event['rain']
    soil = event['soil']
    roll = abs(round(abs(math.atan2(y, z))*57.3)-90)
    pitch = round(abs(math.atan2(-x, math.sqrt(y*y + z*z)))*57.3)
    print('roll: ')
    print(roll)
    print('pitch: ')
    print(pitch)

    insert_entry(event, roll, pitch, epochtime)
    calculate_riskfactor(event, roll, pitch,rain,soil, epochtime)

def insert_entry(event, roll, pitch, timestamp):
    # Insert item into entries_table
    if event['status'] == -1:
        # Sensor has an issue, do not add other parameters
        entries_table.put_item(Item={
            'entryUUID': event['entryUUID'],
            'status': event['status'],
            'deviceID': event['deviceID'],
            'createdDate': timestamp
        })
    elif event['status'] == 0:
        # Sensor ok
        entries_table.put_item(Item={
            'entryUUID': event['entryUUID'],
            'status': event['status'],
            'deviceID': event['deviceID'],
            'accl_x': decimal.Decimal(repr(event['accl_x'])),
            'accl_y': decimal.Decimal(repr(event['accl_y'])),
            'accl_z': decimal.Decimal(repr(event['accl_z'])),
            'gyro_x': decimal.Decimal(repr(event['gyro_x'])),
            'gyro_y': decimal.Decimal(repr(event['gyro_y'])),
            'gyro_z': decimal.Decimal(repr(event['gyro_z'])),
            'roll': roll,
            'pitch': pitch,
            'rain': event['rain'],
            'soil': event['soil'],
            'createdDate': timestamp
        })

# Roll range: [0,180], pitch range: [0,90], soil range: [0,1023]
# riskFactor = d*k1
# where k1 = ((roll + pitch )/90*50+((abs(rain-1023)+abs(soil-1023))/1446*50))/100

def calculate_riskfactor(event, roll, pitch, rain, soil, timestamp):
    risk_factor = 0

    # First step: retrieve deviceID from event
    if event['status'] != 0:
        return #ignore this input as the sensor is not responding
    device_id = event['deviceID']

    # Second step: get 2 previous valid entries from this device ID and compute
    response = entries_table.scan(
        Limit=10,
        FilterExpression=boto3.dynamodb.conditions.Attr('deviceID').eq(device_id) & boto3.dynamodb.conditions.Attr('status').eq(0)
    )
    array = response['Items']
    if len(array) > 1:
        # The database has enough data
        array.sort(key=lambda x: x['createdDate'], reverse=False)
        k1 = ((roll + pitch )/90*50+((abs(rain-1023)+abs(soil-1023))/1446*50))/100
        print('k1: ')
        print(k1)
        if k1 < 1:
            risk_factor=format(k1*100, ".3")
        else:
            risk_factor="100"
            k1=1.000
        sns = boto3.client(service_name="sns")
        topicArn = 'arn:aws:sns:ap-northeast-1:315300711776:dynamodb'
        if k1 > 0.5:
            sns.publish(
            TopicArn = topicArn,
            Message = 'Warning Riskfator is about : '+ risk_factor +'%'+'\n' +'createdDate UNIX timestamp :'+ str(timestamp),

        )
        print('riskFactorSMS: ')
        print(risk_factor)
        risk_factor=format(k1, ".15")
        print('riskFactorDB: ')
        print(risk_factor)



    # Third step: Write results to database
    devices_table.update_item(
        Key={
            'deviceID': device_id
        },
        UpdateExpression='SET riskFactor = :v1, lastUpdated = :v2',
        ExpressionAttributeValues={
            ':v1': risk_factor,
            ':v2': timestamp
        }
    )
