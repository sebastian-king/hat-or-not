package com.hackdfw.hatornot.ui.home

import android.Manifest
import android.app.Activity
import android.app.Activity.RESULT_OK
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.provider.MediaStore
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.View.GONE
import android.view.View.VISIBLE
import android.view.ViewGroup
import android.widget.Button
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.bumptech.glide.Glide
import com.google.gson.Gson
import com.hackdfw.hatornot.databinding.FragmentHomeBinding
import com.hackdfw.hatornot.ui.camera.CameraActivity
import com.hackdfw.hatornot.ui.apiclient.ApiClient
import okhttp3.MediaType
import okhttp3.RequestBody
import retrofit2.Call
import retrofit2.Callback
import retrofit2.Response
import java.io.ByteArrayOutputStream
import java.net.URI


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

        binding.checkMe.setOnClickListener {
            val intent = Intent(requireContext().applicationContext, CameraActivity::class.java)
            startActivityForResult(intent, 666)
        }
        binding.hat.setOnClickListener {
            captureImage(binding.hat)
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

    private fun postSet(body: ByteArray?, header: String, binding: Button) {
        val requestBody: RequestBody? = RequestBody.create(MediaType.get("application/octet-stream"), body)
        val clothesRequest: Call<Void?>? = ApiClient().getService()?.postClothes(header,requestBody)
        clothesRequest?.enqueue(object : Callback<Void?> {
            override fun onFailure(call: Call<Void?>, t: Throwable) {
                t.localizedMessage?.let { Log.e("Failure", it) }
            }
            override fun onResponse(call: Call<Void?>, response: Response<Void?>) {
                Log.e("Success", response.message())
                val gson = Gson()
                //var jsonReader = gson.fromJson<com.hackdfw.hatornot.ui.Response>(response.toString(),com.hackdfw.hatornot.ui.Response::class.java)
                //binding.setBackgroundColor(jsonReader.clothingColour.toInt())
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
                        postSet(stream.toByteArray(),"hat",binding.hat)
                        Log.e("Potato", "1")
                    }
                    binding.glasses->{
                        imageByteArrayGlasses = stream.toByteArray()
                        postSet(stream.toByteArray(),"glasses",binding.glasses)
                        Log.e("Potato", "2")
                    }
                    binding.top->{
                        imageByteArrayTop = stream.toByteArray()
                        postSet(stream.toByteArray(),"top",binding.top)
                        Log.e("Potato", "3")
                    }
                    binding.bottom->{
                        imageByteArrayBottom = stream.toByteArray()
                        postSet(stream.toByteArray(),"bottom",binding.bottom)
                        Log.e("Potato", "4")
                    }
                    binding.shoes->{
                        imageByteArrayShoes = stream.toByteArray()
                        postSet(stream.toByteArray(),"shoes",binding.shoes)
                        Log.e("Potato", "5")
                    }
                }
                imageBitmap.recycle()
            }
        }
        Glide.with(requireContext()).load(requireContext().resources.getIdentifier("rainbow", "drawable",requireContext().packageName)).into(binding.gif)
        binding.gif.visibility = VISIBLE
        Handler(Looper.getMainLooper()).postDelayed({
            binding.gif.visibility = GONE
        }, 3000)
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

}