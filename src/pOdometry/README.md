# pOdometry 
This app is a "pure" MOOS app that at its base form, just calculates and publishes the distance the vessel has traveled since the app's startup.

## Inputs
The app subscribes to NAV_X and NAV_Y. In each loop, "last" buffered coordinates are subtracted from "current" data, and the app uses the pythagorean theorem to find the straight-line distance between the two points.

## Outputs
The straight-line distance between the two points is summed over each loop of the app, and published to the MOOSDB as ODOMETRY_DIST.

If desired, you can also configure `other_odometry_units` to publish additional values in other units, i.e.:

```
other_odometry_units=mi
```

## Configuration
The staleness_threshold is a config for the number of seconds allowed to pass between receipts of NAV coordinates. By default it's 10.

The odometry units supported by default include:
- meters (default)
- miles ("mi")
- kilometers ("km")
- nautical miles ("nmi")

You can also update/add to this app's configs at runtime by posting a `"key=value"` pair to the update variable `ODOMETRY_UPDATES`

## Known issues
The app originally showed some inconsistencies when changing the apptick/commstick in the .moos config. The below experiments were conducted at timewarp=10. The waypoint was at -30, 30 and the vessel started at 0,0. Later, introducing a queue to store buffered coordinates (and then working through the queue in iterate()) *drastically* reduced the issue.

### Without a Queue
- At apptick/commstick=4 the app showed ~90 meters
- At apptick/commstick=8 the app showed ~176 meters
- At apptick/commstick=16 the app showed ~353 meters

### With a Queue
- At apptick/commstick=4 the app showed ~110 meters
- At apptick/commstick=8 the app showed ~110 meters
- At apptick/commstick=16 the app showed ~111 meters