using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using UnityEngine;

public class IMU_Sim : MonoBehaviour
{

    SerialPort stream = new SerialPort("COM6", 115200);
    public string strReceived;
    public const int SIZE = 12;
    public string[] strData = new string[SIZE];
    public string[] strData_received = new string[SIZE];
    public float qw, qx, qy, qz;
    void Start()
    {
        stream.Open(); //Open the Serial Stream.
    }

    // Update is called once per frame
    void Update()
    {
        strReceived = stream.ReadLine(); //Read the information  

        strData = strReceived.Split(',');
        if (strData[0] != "" && strData[1] != "" && strData[2] != "" && strData[3] != "")//make sure data are reday
        {
            for (int i = 0; i < SIZE; i++)
            {
              strData_received[i] = strData[i];
            }



            qw = float.Parse(strData_received[0]);
            qx = float.Parse(strData_received[1]);
            qy = float.Parse(strData_received[2]);
            qz = float.Parse(strData_received[3]);
            
          //  transform.localRotation = new Quaternion(-qy, -qz, qx, qw); // Testing 
            transform.rotation = new Quaternion(-qy, -qz, qx, qw); // Testing 

            /*
          // Convert quaternion to Euler angles
            Quaternion quaternion = new Quaternion(qx, qy, qz, qw);
            Vector3 euler = quaternion.eulerAngles;

            // Apply rotation transformations using Euler angles
            transform.rotation = Quaternion.Euler(-euler.y, -euler.z, euler.x);*/

        }
    }
}