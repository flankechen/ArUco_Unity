// on OpenGL ES there is no way to query texture extents from native texture id
#if (UNITY_IPHONE || UNITY_ANDROID) && !UNITY_EDITOR
	#define UNITY_GLES_RENDERER
#endif


using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;


public class UseRenderingPlugin : MonoBehaviour
{
	// Native plugin rendering events are only called if a plugin is used
	// by some script. This means we have to DllImport at least
	// one function in some active script.
	// For this example, we'll call into plugin's SetTimeFromUnity
	// function and pass the current time so the plugin can animate.

#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
	private static extern void SetTimeFromUnity(float t);

	//native function for open web cam.
#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
	private static extern Boolean OpenWebCam(int index_0, int index_1);

//native function for init a board detector.
#if UNITY_IPHONE && !UNITY_EDITOR
[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin", EntryPoint="init_ArUco_board")]
#endif
	private static extern Boolean Init_ArUco_board();


//native function for init a marker detector.
#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin", EntryPoint="init_ArUco_marker")]
#endif
	private static extern Boolean Init_ArUco_marker();

	//native function call for destroying web cam
#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
	private static extern void DestroyWebCam();

//native function for open web cam.
#if UNITY_IPHONE && !UNITY_EDITOR
[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin", EntryPoint="get_map_x1")]
#endif
	private static extern IntPtr get_map_x1();

//native function for open web cam.
#if UNITY_IPHONE && !UNITY_EDITOR
[DllImport ("__Internal")]
#else
[DllImport ("RenderingPlugin", EntryPoint="get_map_y1")]
#endif
private static extern IntPtr get_map_y1();

//native function gets the translation vector from cpp
#if UNITY_IPHONE && !UNITY_EDITOR
[DllImport ("__Internal")]
#else
[DllImport ("RenderingPlugin", EntryPoint="get_translation")]
#endif
private static extern IntPtr get_translation();

//native function gets the 4 orientation vector from cpp
#if UNITY_IPHONE && !UNITY_EDITOR
[DllImport ("__Internal")]
#else
[DllImport ("RenderingPlugin", EntryPoint="get_orientation")]
#endif
private static extern IntPtr get_orientation();

//native function gets the opengl modelview matrix
#if UNITY_IPHONE && !UNITY_EDITOR
[DllImport ("__Internal")]
#else
[DllImport ("RenderingPlugin", EntryPoint="get_modelview_matrix")]
#endif
private static extern IntPtr get_modelview_matrix();

//native function gets the opengl modelview matrix
#if UNITY_IPHONE && !UNITY_EDITOR
[DllImport ("__Internal")]
#else
[DllImport ("RenderingPlugin", EntryPoint="get_prob")]
#endif
private static extern float get_detect_prob();

	//native function gets the opengl modelview matrix
	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin", EntryPoint="get_id")]
	#endif
	private static extern int get_marker_id();


	// We'll also pass native pointer to a texture in Unity.
	// The plugin will fill texture data from native code.
#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
#if UNITY_GLES_RENDERER
	private static extern void SetTextureFromUnity(System.IntPtr texture, int w, int h);
#else
	private static extern void SetTextureFromUnity(System.IntPtr texture);
#endif


#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport("RenderingPlugin")]
#endif
	private static extern void SetUnityStreamingAssetsPath([MarshalAs(UnmanagedType.LPStr)] string path);


#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport("RenderingPlugin")]
#endif
	private static extern IntPtr GetRenderEventFunc();

	private GameObject StereoCamera;

	private int width = 960;
	private int height = 1080;

	private Texture2D texture;
	private float[] map_x1;
	private float[] map_y1;

	public Boolean init_success = false;
	public UseRenderingPlugin_right render_sc_right;

	private double[] mPosition = new double[3];
	private double[] mOrientation = new double[4];
	private double[] mModelViewMatrix = new double[16];
	private Matrix4x4 W2C_matrix;
	
	IEnumerator Start()
	{
		StereoCamera = GameObject.Find ("StereoCamera");

		if (OpenWebCam (1, 0) && Init_ArUco_board()) {
			Debug.Log("init success!");

			//making a new texture to shader
			texture = new Texture2D (width, height, TextureFormat.RGFloat, false);
			Material mat = GetComponent<Renderer>().material;
			mat.SetTexture ("_Texture2", texture);
			
			//copying the calib params from opencv dll
			map_x1 = new float[width * height];
			map_y1 = new float[width * height];

			IntPtr ptr =  get_map_x1 ();
			Marshal.Copy (ptr, map_x1, 0, width * height);

			ptr = get_map_y1 ();
			Marshal.Copy (ptr, map_y1, 0, width * height);

			Debug.Log("map x1 0: "+map_x1[1000]);
			Debug.Log("map x1 1: "+map_x1[1001]);
			Debug.Log("map x1 2: "+map_x1[2]);
			Debug.Log("map x1 3: "+map_x1[3]);
			Debug.Log("map y1 0: "+map_y1[1000]);
			Debug.Log("map y1 1: "+map_y1[1001]);
			Debug.Log("map y1 2: "+map_y1[2]);
			Debug.Log("map y1 3: "+map_y1[3]);

			for (int i=0; i<width; ++i) {
				for(int j=0; j<height; ++j){
					Color color;
					color.r = map_x1[j*width+i]/(float)width;
					color.g = map_y1[j*width+i]/(float)height;
					//				color.r = ((float)i-0.0f)/(float)width;
					//				color.g = ((float)j-0.0f)/(float)height;
					color.b = 0.0f;
					color.a = 1.0f;
					texture.SetPixel(i,j,color);
				}
			}
			
			texture.Apply ();

			init_success = true;

			render_sc_right.enabled = true;
		} else {
			Debug.Log("init false");
		}

		SetUnityStreamingAssetsPath(Application.streamingAssetsPath);

		CreateTextureAndPassToPlugin();
		yield return StartCoroutine("CallPluginAtEndOfFrames");
	}

	private void CreateTextureAndPassToPlugin()
	{
		// Create a texture
		Texture2D tex = new Texture2D(width,height,TextureFormat.ARGB32,false);
		tex.wrapMode = TextureWrapMode.Clamp;
		// Set point filtering just so we can see the pixels clearly
		tex.filterMode = FilterMode.Bilinear;
		// Call Apply() so it's actually uploaded to the GPU
		tex.Apply();

		// Set texture onto our matrial
		GetComponent<Renderer>().material.mainTexture = tex;

		// Pass texture pointer to the plugin
	#if UNITY_GLES_RENDERER
		SetTextureFromUnity (tex.GetNativeTexturePtr(), tex.width, tex.height);
	#else
		SetTextureFromUnity (tex.GetNativeTexturePtr());
	#endif
	}


	//reform a left handed system transform matrix from right handed
	public static Matrix4x4 LHMatrixFromRHMatrix(Matrix4x4 rhm)
	{
		Matrix4x4 lhm = new Matrix4x4();;
		
		// Column 0.
		lhm[0, 0] =  rhm[0, 0];
		lhm[1, 0] =  rhm[1, 0];
		lhm[2, 0] = -rhm[2, 0];
		lhm[3, 0] =  rhm[3, 0];
		
		// Column 1.
		lhm[0, 1] =  rhm[0, 1];
		lhm[1, 1] =  rhm[1, 1];
		lhm[2, 1] = -rhm[2, 1];
		lhm[3, 1] =  rhm[3, 1];
		
		// Column 2.
		lhm[0, 2] = -rhm[0, 2];
		lhm[1, 2] = -rhm[1, 2];
		lhm[2, 2] =  rhm[2, 2];
		lhm[3, 2] = -rhm[3, 2];
		
		// Column 3.
		lhm[0, 3] =  rhm[0, 3];
		lhm[1, 3] =  rhm[1, 3];
		lhm[2, 3] = -rhm[2, 3];
		lhm[3, 3] =  rhm[3, 3];
		
		return lhm;
	}

	//get position from transform matrix
	public static Vector3 PositionFromMatrix(Matrix4x4 m)
	{
		return m.GetColumn(3);
	}
	
	//get rotation quaternion from matrix
	public static Quaternion QuaternionFromMatrix(Matrix4x4 m)
	{
		// Trap the case where the matrix passed in has an invalid rotation submatrix.
		if (m.GetColumn(2) == Vector4.zero) {
			Debug.Log("QuaternionFromMatrix got zero matrix.");
			return Quaternion.identity;
		}
		return Quaternion.LookRotation(m.GetColumn(2), m.GetColumn(1));
	}

	private IEnumerator CallPluginAtEndOfFrames()
	{
		while (true) {

//			StereoCamera.transform.Rotate(Vector3.up * 50f * Time.deltaTime);

			float time1 = Time.realtimeSinceStartup;

			// Wait until all frame rendering is done
			yield return new WaitForEndOfFrame();

			// Set time for the plugin
			SetTimeFromUnity (Time.timeSinceLevelLoad);

			// Issue a plugin event with arbitrary integer identifier.
			// The plugin can distinguish between different
			// things it needs to do based on this ID.
			// For our simple plugin, it does not matter which ID we pass here.
			GL.IssuePluginEvent(GetRenderEventFunc(), 0);

			float time2 = Time.realtimeSinceStartup;
			float interval = time2 - time1;
//			Debug.Log("time_left: "+interval*1000+"ms.");

//			IntPtr PositionPtr = get_translation();
//			Marshal.Copy (PositionPtr, mPosition, 0, 3);
//			Debug.Log("position 0 :"+mPosition[0]);
//			Debug.Log("position 1 :"+mPosition[1]);
//			Debug.Log("position 2 :"+mPosition[2]);
////
//			IntPtr OrientationPtr = get_orientation();
//			Marshal.Copy (OrientationPtr, mOrientation, 0, 4);
//			Debug.Log("orientation 0 :"+mOrientation[0]);
//			Debug.Log("orientation 1 :"+mOrientation[1]);
//			Debug.Log("orientation 2 :"+mOrientation[2]);
//			Debug.Log("orientation 2 :"+mOrientation[3]);

			IntPtr ModelViewMatrixPtr = get_modelview_matrix();
			Marshal.Copy (ModelViewMatrixPtr, mModelViewMatrix, 0, 16);
//			Debug.Log("modelview 0 :"+mModelViewMatrix[0]);
//			Debug.Log("modelview 1 :"+mModelViewMatrix[1]);
//			Debug.Log("modelview 2 :"+mModelViewMatrix[2]);
//			Debug.Log("modelview 3 :"+mModelViewMatrix[3]);
//			Debug.Log("modelview 4 :"+mModelViewMatrix[4]);
//			Debug.Log("modelview 5 :"+mModelViewMatrix[5]);
//			Debug.Log("modelview 6 :"+mModelViewMatrix[6]);
//			Debug.Log("modelview 7 :"+mModelViewMatrix[7]);
//			Debug.Log("modelview 8 :"+mModelViewMatrix[8]);
//			Debug.Log("modelview 9 :"+mModelViewMatrix[9]);
//			Debug.Log("modelview 10 :"+mModelViewMatrix[10]);
//			Debug.Log("modelview 11 :"+mModelViewMatrix[11]);
//			Debug.Log("modelview 12 :"+mModelViewMatrix[12]);
//			Debug.Log("modelview 13 :"+mModelViewMatrix[13]);
//			Debug.Log("modelview 14 :"+mModelViewMatrix[14]);
//			Debug.Log("modelview 15 :"+mModelViewMatrix[15]);

			//transfrom to column based
			W2C_matrix.m00 = (float)mModelViewMatrix [0];
			W2C_matrix.m01 = (float)mModelViewMatrix [4];
			W2C_matrix.m02 = (float)mModelViewMatrix [8];
			W2C_matrix.m03 = (float)mModelViewMatrix[12];
			W2C_matrix.m10 = (float)mModelViewMatrix [1];
			W2C_matrix.m11 = (float)mModelViewMatrix [5];
			W2C_matrix.m12 = (float)mModelViewMatrix [9];
			W2C_matrix.m13 = (float)mModelViewMatrix [13];
			W2C_matrix.m20 = (float)mModelViewMatrix [2];
			W2C_matrix.m21 = (float)mModelViewMatrix [6];
			W2C_matrix.m22 = (float)mModelViewMatrix [10];
			W2C_matrix.m23 = (float)mModelViewMatrix [14];
			W2C_matrix.m30 = 0;
			W2C_matrix.m31 = 0;
			W2C_matrix.m32 = 0;
			W2C_matrix.m33 = 1;

			Matrix4x4 transformationMatrix = LHMatrixFromRHMatrix(W2C_matrix);
			Matrix4x4 pose = transformationMatrix.inverse;

			Vector3 arPosition = PositionFromMatrix(pose);
			Quaternion arRotation = QuaternionFromMatrix(pose);

			StereoCamera.transform.localPosition = arPosition;
			StereoCamera.transform.localRotation = arRotation;
//			StereoCamera.transform.Rotate (Vector3.up * 180f);
			StereoCamera.transform.Rotate (Vector3.forward*180f);

			float detect_pro = get_detect_prob();
			Debug.Log("detect prob :"+detect_pro);
//
			int marker_id = get_marker_id();
			Debug.Log("marker id :"+marker_id);

        }
	}

	void OnApplicationQuit()
	{
		DestroyWebCam ();
		Debug.Log("quit");
	}
}
