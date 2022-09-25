package com.hackdfw.hatornot.ui.home

import android.Manifest
import android.app.Activity
import android.app.Activity.RESULT_OK
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.os.Bundle
import android.provider.MediaStore
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.hackdfw.hatornot.databinding.FragmentHomeBinding
import com.hackdfw.hatornot.ui.CameraActivity
import com.hackdfw.hatornot.ui.apiclient.ApiClient
import okhttp3.MediaType
import okhttp3.RequestBody
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import java.io.ByteArrayOutputStream


class HomeFragment : Fragment() {

    private var _binding: FragmentHomeBinding? = null
    private var imageByteArrayHat: ByteArray? = null
    private var imageByteArrayGlasses: ByteArray? = null
    private var imageByteArrayTop: ByteArray? = null
    private var imageByteArrayBottom: ByteArray? = null
    private var imageByteArrayShoes: ByteArray? = null
    var buttonPressed: Button? = null

    private val binding get() = _binding!!
    private val CAPTURE_IMAGE_ACTIVITY_REQUEST_CODE = 100

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentHomeBinding.inflate(inflater, container, false)

        binding.hat.setOnClickListener {
            //captureImage(binding.hat)
            val intent = Intent(requireContext().applicationContext,CameraActivity::class.java)
            startActivityForResult(intent, 666)

        }
        binding.glasses.setOnClickListener {
            captureImage(binding.glasses)
        }
        binding.top.setOnClickListener {
            captureImage(binding.top)
        }
        binding.bottom.setOnClickListener {
            captureImage(binding.bottom)
        }
        binding.shoes.setOnClickListener {
            captureImage(binding.shoes)
        }

        return binding.root
    }

    private fun postSet(body: ByteArray?, header: String) {
        val requestBody: RequestBody? = RequestBody.create(MediaType.get("application/octet-stream"), body)
        val clothesRequest: Call<Void?>? = ApiClient().getService()?.postClothes(header,requestBody)
        clothesRequest?.enqueue(object : Callback<Void?> {
            override fun onFailure(call: Call<Void?>, t: Throwable) {
                t.localizedMessage?.let { Log.e("Failure", it) }
            }
            override fun onResponse(call: Call<Void?>, response: Response<Void?>) {
                Log.e("Success", "DONE")
            }
        })
    }

    private fun captureImage(binding: Button) {
        if (ContextCompat.checkSelfPermission(
                requireContext(),
                Manifest.permission.CAMERA
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                requireContext() as Activity,
                arrayOf(Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE),
                0
            )
        } else {
            val intent = Intent(MediaStore.ACTION_IMAGE_CAPTURE)
            buttonPressed = binding
            try {
                startActivityForResult(intent, CAPTURE_IMAGE_ACTIVITY_REQUEST_CODE)
                } catch (e: Exception) {
                    Log.e("Exception", e.message.toString())
                }
            }
        }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == CAPTURE_IMAGE_ACTIVITY_REQUEST_CODE) {
            if (resultCode == RESULT_OK) {
                val imageBitmap = data?.extras?.get("data") as Bitmap
                val stream = ByteArrayOutputStream()
                imageBitmap.compress(Bitmap.CompressFormat.JPEG, 90, stream)
                Log.e("Potato", buttonPressed.toString())
                when(buttonPressed){
                    binding.hat->{
                        imageByteArrayHat = stream.toByteArray()
                        postSet(stream.toByteArray(),"hat")
                        Log.e("Potato", "1")
                    }
                    binding.glasses->{
                        imageByteArrayGlasses = stream.toByteArray()
                        postSet(stream.toByteArray(),"glasses")
                        Log.e("Potato", "2")
                    }
                    binding.top->{
                        imageByteArrayTop = stream.toByteArray()
                        postSet(stream.toByteArray(),"top")
                        Log.e("Potato", "3")
                    }
                    binding.bottom->{
                        imageByteArrayBottom = stream.toByteArray()
                        postSet(stream.toByteArray(),"bottom")
                        Log.e("Potato", "4")
                    }
                    binding.shoes->{
                        imageByteArrayShoes = stream.toByteArray()
                        postSet(stream.toByteArray(),"shoes")
                        Log.e("Potato", "5")
                    }
                }
                imageBitmap.recycle()
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

}