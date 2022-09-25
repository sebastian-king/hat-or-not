package com.hackdfw.hatornot.ui.apiclient

import okhttp3.RequestBody
import retrofit2.Call
import retrofit2.http.Body
import retrofit2.http.Header
import retrofit2.http.POST

interface Service {
    @POST("ingest/outfit")
    fun postClothes(@Header("clothing-type") header: String,@Body body: RequestBody?): Call<Void?>?
}