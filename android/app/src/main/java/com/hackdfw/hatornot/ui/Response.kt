package com.hackdfw.hatornot.ui

data class Response (
    var shouldWearHat: Boolean,
    var isWearingHat: Boolean,
    var clothingColour: String,
    var hatMetadata: HatMetaData
    )