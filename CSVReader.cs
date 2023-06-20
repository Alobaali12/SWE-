using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CSVReader : MonoBehaviour
{
    public TextAsset textAssetData;
    public int Samples_size = 0;
    public float Units = 1;
    public float rotationDuration = 1f; // Define a rotation duration in seconds
    private int currentTimeIndex = 0; // Track the current time index
    private float elapsedTime = 0f;  // Track the elapsed time since the rotation started
    public int rotation_tune = 0;
    public Vector3 Init_position; // Initial object position  
    public Vector3 Init_Angles; // Initial object rotation
    public float frame_time = 1f;

    [System.Serializable]

    public class Samples
    {
        public float time;
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
        ///   transform.position = new Vector3( (float)-1000, (float)-1000, (float)-23.40332); testing
        Time.timeScale = 1f; // Set the time scale to 0.5 for slower animation
        Init_position = transform.position;
        Init_Angles = transform.eulerAngles;
        Debug.Log("Start Initial Position: " + transform.position);
        ReadCSV();
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

        // Check if we have more samples to rotate
        Debug.Log("currentTimeIndex: " + currentTimeIndex.ToString());
        if (currentTimeIndex < myWelderList.samples.Length - 1)
        {
            // Increase the elapsed time
            elapsedTime += Time.deltaTime;

            // Calculate the interpolation factor based on the elapsed time and duration
            float t = elapsedTime / rotationDuration;

            // Interpolate between the current and next sample based on time
            Quaternion rotation = Quaternion.Slerp(
                Quaternion.Euler(Init_Angles.x + myWelderList.samples[currentTimeIndex].pitch, -(Init_Angles.y + myWelderList.samples[currentTimeIndex].yaw), Init_Angles.z + myWelderList.samples[currentTimeIndex].roll),
                Quaternion.Euler(Init_Angles.x + myWelderList.samples[currentTimeIndex + 1].pitch, -(Init_Angles.y + myWelderList.samples[currentTimeIndex + 1].yaw), Init_Angles.z + myWelderList.samples[currentTimeIndex + 1].roll),
                t
            );

            // Apply the rotation to the object's transform, testing: 
            // rotation = Quaternion.Euler(rotation_tune + myWelderList.samples[currentTimeIndex].pitch,-( rotation_tune + myWelderList.samples[currentTimeIndex].yaw), rotation_tune + myWelderList.samples[currentTimeIndex].roll);
            // transform.rotation = rotation;
            transform.localRotation = rotation;



            // Calculate the position based on CTDW and HD values
            Vector3 position = transform.position;
            position.z = Init_position.z + myWelderList.samples[currentTimeIndex].CTDW / Units;
            position.x = Init_position.x - ( myWelderList.samples[0].HD - myWelderList.samples[currentTimeIndex].HD )/ Units;
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

    void ReadCSV()
    {
        int attributes_size = 8;
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
            float.TryParse(data[startIndex + 1], out myWelderList.samples[i].pitch);
            float.TryParse(data[startIndex + 2], out myWelderList.samples[i].roll);
            float.TryParse(data[startIndex + 3], out myWelderList.samples[i].yaw);
            float.TryParse(data[startIndex + 4], out myWelderList.samples[i].CTDW);
            float.TryParse(data[startIndex + 5], out myWelderList.samples[i].HD);
            float.TryParse(data[startIndex + 6], out myWelderList.samples[i].HV);
            float.TryParse(data[startIndex + 7], out myWelderList.samples[i].Mic);
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
}
