package com.hackdfw.hatornot.ui.camera

import android.app.Activity
import android.app.AlertDialog
import android.content.DialogInterface
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.hardware.Camera
import android.os.Bundle
import android.util.Log
import android.widget.Button
import android.widget.FrameLayout
import com.google.gson.Gson
import com.hackdfw.hatornot.R
import com.hackdfw.hatornot.ui.apiclient.ApiClient
import okhttp3.MediaType
import okhttp3.RequestBody
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response


class CameraActivity : Activity() {

    private var mCamera: Camera? = null
    private var mPreview: CameraPreview? = null
    private val mPicture = Camera.PictureCallback { data,_ ->
        postSet(data,"custom")
        val intent = Intent()
        intent.putExtra("","")
        setResult(666,intent)
        mCamera?.release()
        finish()
    }


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.camera_activity)

        // Create an instance of Camera
        mCamera = getCameraInstance()

        mPreview = mCamera?.let {
            // Create our Preview view
            CameraPreview(this, it)
        }

        // Set the Preview view as the content of our activity.
        mPreview?.also {
            val preview: FrameLayout = findViewById(R.id.camera_preview)
            preview.addView(it)
        }

        val captureButton: Button = findViewById(R.id.button_capture)
        captureButton.setOnClickListener {
            // get an image from the camera
            mCamera?.takePicture(null, null, mPicture)
        }
    }

    private fun getCameraInstance(): Camera? {
        return try {
            Camera.open() // attempt to get a Camera instance
        } catch (e: Exception) {
            // Camera is not available (in use or does not exist)
            null // returns null if camera is unavailable
        }
    }

    private fun convertCompressedByteArrayToBitmap(src: ByteArray): Bitmap? {
        return BitmapFactory.decodeByteArray(src, 0, src.size)
    }

    private fun postSet(body: ByteArray?, header: String) {
        val requestBody: RequestBody? = RequestBody.create(MediaType.get("application/octet-stream"), body)
        val clothesRequest: Call<Void?>? = ApiClient().getService()?.postClothes(header,requestBody)
        clothesRequest?.enqueue(object : Callback<Void?> {
            override fun onFailure(call: Call<Void?>, t: Throwable) {
                t.localizedMessage?.let { Log.e("Failure", it) }
            }
            override fun onResponse(call: Call<Void?>, response: Response<Void?>) {
                if (null != response){
                    val gson = Gson()
                    var jsonReader = gson.fromJson<com.hackdfw.hatornot.ui.Response>(response.toString(),com.hackdfw.hatornot.ui.Response::class.java)
                    val dialog = AlertDialog.Builder(applicationContext)
                    dialog.setTitle("Hat or Not?")
                    if (jsonReader.isWearingHat){
                        dialog.setMessage("You are wearing a ${jsonReader.hatMetadata.type} hat " +
                                "of color ${jsonReader.clothingColour}")
                    }else{
                        dialog.setMessage("You are not wearing a hat :(")
                    }

                    dialog.setPositiveButton("OK", DialogInterface.OnClickListener { dialog, which ->
                        dialog.dismiss()
                    })
                    dialog.show()
                }
            }
        })
    }
}