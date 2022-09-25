package com.hackdfw.hatornot.ui.apiclient

import okhttp3.Credentials
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory

class ApiClient {
    private fun getRetrofit(): Retrofit {
        val httpLoggingInterceptor = HttpLoggingInterceptor()
        httpLoggingInterceptor.level = HttpLoggingInterceptor.Level.BODY
        val okHttpClient = OkHttpClient.Builder()
        okHttpClient.addInterceptor { chain ->
            val original: Request = chain.request()

            // String credentials = Credentials.basic(username, password);
           // val credentials: String = Credentials.basic("  ", "   ")
            val request: Request = original.newBuilder()
                //.header("   ", credentials)
                .method(original.method(), original.body())
                .build()
            chain.proceed(request)
        }
        val client = okHttpClient.build()
        return Retrofit.Builder()
            .baseUrl("http://hat-or-not.helpfulseb.com:8080/") //Emulator
            .addConverterFactory(GsonConverterFactory.create())
            .client(client)
            .build()
    }

    fun getService(): Service? {
        return getRetrofit().create(Service::class.java)
    }
}