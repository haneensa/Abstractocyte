# Avizo-Script-Object V3.0 

$this proc constructor {} {
    $this newPortDoIt "portDoIt"
}

$this proc exportCenterline { } {
    set filename [file rootname ["Time Series Control" getCurrentFilename]]
    append filename ".am"
    ["Centerline Tree" getResult] exportData "Avizo ascii SpatialGraph" $filename
}

$this proc compute { } {
    if { [$this "portDoIt" wasHit] == 1 } {
        lassign ["Time Series Control" time getMinMax] min max
        for { set k $min } { $k < $max } { incr k } {

            "Centerline Tree" fire
            "Centerline Tree" doIt
            "Centerline Tree" fire
            "Centerline Tree" compute
            "Centerline Tree" fire

            $this exportCenterline

            "Time Series Control" fire
            "Time Series Control" time step -f
            "Time Series Control" fire
        }
	   "Centerline Tree" fire
        "Centerline Tree" doIt
        "Centerline Tree" fire
        "Centerline Tree" compute
        "Centerline Tree" fire
        $this exportCurrentFile
    }
}
