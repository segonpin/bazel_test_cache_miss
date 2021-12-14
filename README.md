# Show cache miss after consecutive invocations

First create an empty folder in /tmp to use a disk_cache (to emulate remote cache)
```
$ rm -rf /tmp/bazel_disk_cache && mkdir /tmp/bazel_disk_cache
```

Then we run the test for the first time. It will run 7 actions, of which 4 are linux-sandbox. Notice that the test takes 5 seconds to run.
```
$ bazelisk clean; bazelisk test //... --disk_cache=/tmp/bazel_disk_cache --execution_log_json_file=exec1.json
INFO: Starting clean (this may take a while). Consider using --async if the clean takes more than several minutes.
INFO: Invocation ID: ee4fb2d7-12e7-4f30-9462-0711c5551b1f
INFO: Analyzed target //:sleep_test (46 packages loaded, 571 targets configured).
INFO: Found 1 test target...
Target //:sleep_test up-to-date:
  bazel-bin/sleep_test
INFO: Elapsed time: 5.378s, Critical Path: 5.09s
INFO: 7 processes: 3 internal, 4 linux-sandbox.
INFO: Build completed successfully, 7 total actions
//:sleep_test                                                            PASSED in 5.0s

Executed 1 out of 1 test: 1 test passes.
INFO: Build completed successfully, 7 total actions
```

Then we run the test for the second time. It will run 7 actions, and now only 1 is linux-sandbox. None should be. The reason for the cache-miss is that the test duration is used as input for the missed action, and because the test was cached now, the duration has changed from 5 seconds to 0 seconds, so the commandArgs was different for the action.
```
$ bazelisk clean; bazelisk test //... --disk_cache=/tmp/bazel_disk_cache --execution_log_json_file=exec2.json
INFO: Starting clean (this may take a while). Consider using --async if the clean takes more than several minutes.
INFO: Invocation ID: be5bb58e-0563-4282-ab44-e283f2cabb1d
INFO: Analyzed target //:sleep_test (46 packages loaded, 571 targets configured).
INFO: Found 1 test target...
Target //:sleep_test up-to-date:
  bazel-bin/sleep_test
INFO: Elapsed time: 0.369s, Critical Path: 0.03s
INFO: 7 processes: 3 disk cache hit, 3 internal, 1 linux-sandbox.
INFO: Build completed successfully, 7 total actions
//:sleep_test                                                   (cached) PASSED in 0.0s

Executed 0 out of 1 test: 1 test passes.
INFO: Build completed successfully, 7 total actions
```

Now we run the test again, and now the test run in 0 seconds, as the previous time, so that the action printing the xml report get a cache hit from before.
```
$ bazelisk clean; bazelisk test //... --disk_cache=/tmp/bazel_disk_cache --execution_log_json_file=exec3.json
INFO: Starting clean (this may take a while). Consider using --async if the clean takes more than several minutes.
INFO: Invocation ID: 1a7fe71e-9815-46c5-93f4-4013caf34ad8
INFO: Analyzed target //:sleep_test (46 packages loaded, 571 targets configured).
INFO: Found 1 test target...
Target //:sleep_test up-to-date:
  bazel-bin/sleep_test
INFO: Elapsed time: 0.312s, Critical Path: 0.01s
INFO: 7 processes: 4 disk cache hit, 3 internal.
INFO: Build completed successfully, 7 total actions
//:sleep_test                                                   (cached) PASSED in 0.0s

Executed 0 out of 1 test: 1 test passes.
INFO: Build completed successfully, 7 total actions
```

The action getting the cache miss runs
```
"commandArgs": ["external/bazel_tools/tools/test/generate-xml.sh", "bazel-out/k8-fastbuild/testlogs/sleep_test/test.log", "bazel-out/k8-fastbuild/testlogs/sleep_test/test.xml", "0", "0"],
```
And the generated test report looks like follows
```
$ cat bazel-out/k8-fastbuild/testlogs/sleep_test/test.xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites>
  <testsuite name="sleep_test" tests="1" failures="0" errors="0">
    <testcase name="sleep_test" status="run" duration="0" time="0"></testcase>
      <system-out>
Generated test.log (if the file is not UTF-8, then this may be unreadable):
<![CDATA[exec ${PAGER:-/usr/bin/less} "$0" || exit 1
Executing tests from //:sleep_test
-----------------------------------------------------------------------------]]>
      </system-out>
    </testsuite>
</testsuites>
```

In my opinion the test report should be always the first one (with the duration="5"), or otherwise the last action should not be remoteCacheable.
