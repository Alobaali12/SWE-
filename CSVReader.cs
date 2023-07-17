using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using UnityEngine;

public class CSVReader : MonoBehaviour
{
    public TextAsset textAssetData;
    public int Samples_size = 0;
    public float Units = 2.38f;
    public float CTDW_Units = 2.38f;
    public float rotationDuration = 1f; // Define a rotation duration in seconds
    private int currentTimeIndex = 0; // Track the current time index
    private float elapsedTime = 0f;  // Track the elapsed time since the rotation started
    public int rotation_tune = 0;
    public Vector3 Init_position; // Initial object position  
    public Vector3 Init_Angles; // Initial object rotation
    public Quaternion Init_Rotation;
    public float frame_time = 0.3f;
    public bool SYSTEM_RUN = true;  // if true (ticked box) then regenerate motion using a csv file, if false use realtime serial port data.
    public bool Use_Euler = true;
    public float CTWD_Calibration = 105f; // Distance between tip and Sensor (5 cm)
    // Serial port
    public string COM_PORT = "COM6";
    public int BAUD_RATE = 9600;
    // Real time data read
    SerialPort stream;
    const int SIZE = 12;
    string[] strData = new string[SIZE];
    string[] strData_received = new string[SIZE];
    float qw, qx, qy, qz;
    float pitch, roll, yaw;
    Quaternion Init_Quat;
    float Init_Yaw_Comp = 0; // Initial Yaw compensation (Error deviation)
    int i = 0;
    //
    [System.Serializable]

    public class Samples
    {
        public float time;
        public float w;
        public float x;
        public float y;
        public float z;
        public float pitch;
        public float roll;
        public float yaw;
        public float CTDW;
        public float HD; // Horizontal displacement, velcoity is a filter differentiation of this variable.
        public float HV; // Horizontal Velocity.
        public float Mic; // Sound
    }
    [System.Serializable]

    public class Welder
    {
        public Samples[] samples;
    }

    public Welder myWelderList = new Welder();
    // Start is called before the first frame  update
    void Start()
    {
        if(SYSTEM_RUN)
        {
            Debug.Log("Running CSV file");
            ///   transform.position = new Vector3( (float)-1000, (float)-1000, (float)-23.40332); testing
            Time.timeScale = 1f; // Set the time scale to 0.5 for slower animation
            Init_position = transform.position;
            Init_Angles = transform.eulerAngles;
            Init_Rotation = transform.rotation;
            Debug.Log("Start Initial Position: " + transform.position);
            ReadCSV();
            
            Init_Quat = new Quaternion(myWelderList.samples[0].x, myWelderList.samples[0].z, myWelderList.samples[0].y, myWelderList.samples[0].w);
            Init_Yaw_Comp = Init_Quat.eulerAngles.y - 180f;
        }
        else
        {
            stream = new SerialPort(COM_PORT, BAUD_RATE);
            stream.Open();
            Init_Rotation = transform.rotation;
        }

    }

    // Update is called once per frame
    /*
    void Update()
    {

        for(int i = 0; i < Samples_size; i++)
        {
            Quaternion rotation = Quaternion.Euler(myWelderList.samples[i].pitch, myWelderList.samples[i].yaw, myWelderList.samples[i].roll); // All angles are in degrees
                                                                                                                                              // Apply the rotation to the object's transform
            transform.rotation = rotation;
        }
        // Create a rotation quaternion using Euler angles
    }
    */
    void Update()
    {
        if(SYSTEM_RUN)
        {
            // Check if we have more samples to rotate
            
            if (currentTimeIndex < myWelderList.samples.Length - 1)
            {
                Debug.Log("currentTimeIndex: " + currentTimeIndex.ToString());
                // Increase the elapsed time
                elapsedTime += Time.deltaTime;

                // Calculate the interpolation factor based on the elapsed time and duration
                float t = elapsedTime / rotationDuration;

                /*
                // Interpolate between the current and next sample based on time
                Quaternion rotation = Quaternion.Slerp(
                    Quaternion.Euler(Init_Angles.x + myWelderList.samples[currentTimeIndex].pitch, -(Init_Angles.y + myWelderList.samples[currentTimeIndex].yaw), Init_Angles.z + myWelderList.samples[currentTimeIndex].roll),
                    Quaternion.Euler(Init_Angles.x + myWelderList.samples[currentTimeIndex + 1].pitch, -(Init_Angles.y + myWelderList.samples[currentTimeIndex + 1].yaw), Init_Angles.z + myWelderList.samples[currentTimeIndex + 1].roll),
                    t
                ); 
                */

                Quaternion Current_Quaternion = new Quaternion(myWelderList.samples[currentTimeIndex].y, -myWelderList.samples[currentTimeIndex].z, myWelderList.samples[currentTimeIndex].x, myWelderList.samples[currentTimeIndex].w);
                Quaternion Next_Quaternion = new Quaternion(myWelderList.samples[currentTimeIndex + 1].y, -myWelderList.samples[currentTimeIndex + 1].z, myWelderList.samples[currentTimeIndex + 1].x, myWelderList.samples[currentTimeIndex + 1].w);
                
               Quaternion rotation = Quaternion.Slerp(Current_Quaternion, Next_Quaternion, t);

               rotation.Normalize();

               rotation.w = Mathf.Floor(rotation.w * 1000) / 1000f;
               rotation.x = Mathf.Floor(rotation.x * 1000) / 1000f;
               rotation.y = Mathf.Floor(rotation.y * 1000) / 1000f;
               rotation.z = Mathf.Floor(rotation.z * 1000) / 1000f;
               



                Vector3 Euler_Rotation = rotation.eulerAngles; // Testing 
             //   Euler_Rotation.x -= 180f;
             //   Euler_Rotation.z -= 180;
                Debug.Log("Pitch Angle: " + (360 - Euler_Rotation.x).ToString());


                // Quaternion simulation compensation
              //  Quaternion Rotation_Comp = Quaternion.AngleAxis(-180f, Vector3.left);

           //     rotation = Quaternion.Euler(Euler_Rotation.x, Euler_Rotation.y - Mathf.Abs(Init_Yaw_Comp), Euler_Rotation.z);


                // Apply the rotation to the object's transform, testing: 
                // rotation = Quaternion.Euler(rotation_tune + myWelderList.samples[currentTimeIndex].pitch,-( rotation_tune + myWelderList.samples[currentTimeIndex].yaw), rotation_tune + myWelderList.samples[currentTimeIndex].roll);
                // transform.rotation = rotation;
                if(Use_Euler)
                {
                    transform.localRotation = Quaternion.Euler(Init_Angles.x - myWelderList.samples[currentTimeIndex].pitch, -(Init_Angles.y + myWelderList.samples[currentTimeIndex].yaw), Init_Angles.z + myWelderList.samples[currentTimeIndex].roll);
                }
                else
                {
                    Debug.Log("w,x,y,z, mag: " + rotation.w.ToString() + ", " + rotation.x.ToString() + ", " + rotation.y.ToString() + ", " + rotation.z.ToString() + ", " + Mathf.Sqrt(rotation.w * rotation.w + rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z).ToString());
                  //  transform.localRotation = rotation ; // * Rotation_Comp;
                    transform.rotation = rotation;
                }
                


                // Calculate the position based on CTDW and HD values
              //  Vector3 position = transform.localPosition;
               Vector3 position = transform.position;
                position.z = Init_position.z + ( (myWelderList.samples[currentTimeIndex].CTDW - CTWD_Calibration) / CTDW_Units) * Mathf.Cos(Mathf.Deg2Rad * (360 - Euler_Rotation.x));
                position.y = Init_position.y + ( (myWelderList.samples[currentTimeIndex].CTDW - CTWD_Calibration)  / CTDW_Units) * Mathf.Sin(Mathf.Deg2Rad * (360 - Euler_Rotation.x));
                position.x = Init_position.x - ( myWelderList.samples[0].HD - myWelderList.samples[currentTimeIndex].HD )/ Units;
              //  transform.localPosition = position;
                transform.position = position;

                // Check if the interpolation is complete
                if (t >= frame_time)
                {
                    // Reset the elapsed time
                    elapsedTime = 0f;

                    // Move to the next time index
                    currentTimeIndex++;
                }
            }
        }
        else
        {
            if (stream == null || !stream.IsOpen)
            return;

                if (stream.BytesToRead > 0)
                {
                    string strReceived = stream.ReadLine();
                    Debug.Log(strReceived);
                    if (strReceived != null)
                    {
                        strData = strReceived.Split(',');

                        if (strData.Length >= SIZE)
                        {
                            for (int i = 0; i < SIZE; i++)
                            {
                                strData_received[i] = strData[i];
                                
                            }

                            if (float.TryParse(strData_received[1], out qw) &&
                                float.TryParse(strData_received[2], out qx) &&
                                float.TryParse(strData_received[3], out qy) &&
                                float.TryParse(strData_received[4], out qz) &&
                                float.TryParse(strData_received[5], out roll) &&
                                float.TryParse(strData_received[6], out pitch) &&
                                float.TryParse(strData_received[7], out yaw))
                            {
                                Debug.Log("Index: " + i.ToString());
                                i++;
                                Debug.Log("w,x,y,z: " + qw.ToString() + ", " + qx.ToString() + ", " + qy.ToString() + ", " + qz.ToString());
                               
                                 // Switch the order of qx and qy to represent roll instead of yaw
                                Quaternion quaternion = new Quaternion(qy, -qz, qx, qw); // Swap qx and qy
                                Vector3 Euler = quaternion.eulerAngles;
                           //     Euler.x -= 180f;
                             //   Euler.y -= 90;
                             //   quaternion = Quaternion.Euler(-Euler.x, Euler.y, Euler.z);

                                if(Use_Euler)
                                {
                                    transform.localRotation = Quaternion.Euler(Init_Angles.x - pitch, -(Init_Angles.y + yaw), Init_Angles.z + roll);
                                }
                                else
                                {
                                    // transform.localRotation = quaternion;
                                       transform.rotation = quaternion;
                                }
                            }
                        }
                    }
                }
        }
        }

    void ReadCSV()
    {
        int attributes_size = 12;
        string[] data = textAssetData.text.Split(new string[] { ",", "\n" }, System.StringSplitOptions.RemoveEmptyEntries);
        int tableSize = (data.Length / attributes_size) - 1;
        Samples_size = tableSize;
        myWelderList.samples = new Samples[tableSize];
        Debug.Log("Data length: " + data.Length.ToString());

        // just debugging the string data
        //for (int i = 0; i < data.Length; i++)
        //{
        //    Debug.Log("Data[" + i + "]: " + data[i]);
        //}

        for (int i = 0; i < tableSize; i++)
        {
            int startIndex = attributes_size * (i + 1);

            myWelderList.samples[i] = new Samples();
            float.TryParse(data[startIndex], out myWelderList.samples[i].time);
            float.TryParse(data[startIndex + 1], out myWelderList.samples[i].w);
            float.TryParse(data[startIndex + 2], out myWelderList.samples[i].x);
            float.TryParse(data[startIndex + 3], out myWelderList.samples[i].y);
            float.TryParse(data[startIndex + 4], out myWelderList.samples[i].z);
            float.TryParse(data[startIndex + 5], out myWelderList.samples[i].roll);
            float.TryParse(data[startIndex + 6], out myWelderList.samples[i].pitch);
            float.TryParse(data[startIndex + 7], out myWelderList.samples[i].yaw);
            float.TryParse(data[startIndex + 8], out myWelderList.samples[i].CTDW);
            float.TryParse(data[startIndex + 9], out myWelderList.samples[i].HD);
            float.TryParse(data[startIndex + 10], out myWelderList.samples[i].HV);
            float.TryParse(data[startIndex + 11], out myWelderList.samples[i].Mic);
            // Output the values to the console
            Debug.Log("Time: " + myWelderList.samples[i].time.ToString()
                + ", Pitch: " + myWelderList.samples[i].pitch.ToString()
                + ", Roll: " + myWelderList.samples[i].roll.ToString()
                + ", Yaw: " + myWelderList.samples[i].yaw.ToString()
                + ", CTDW: " + myWelderList.samples[i].CTDW.ToString()
                + ", HD: " + myWelderList.samples[i].HD.ToString()
                + ", HV: " + myWelderList.samples[i].HV.ToString()
                + ", Mic: " + myWelderList.samples[i].Mic.ToString());
        }
    }
    void OnDestroy()
    {
    if (stream != null && stream.IsOpen)
    {
        stream.Close();
    }
    }
}
