#!/usr/bin/python

import urllib2

def test_server(url):
    success_count = 0
    failure_count = 0

    for i in range(1000):
        try:
            response = urllib2.urlopen(url)
            status_code = response.getcode()
            if status_code == 200:
                success_count += 1
                print("Request #{0}: Success".format(i + 1))
            else:
                failure_count += 1
                print("Request #{0}: Failed, Status Code {1}".format(i + 1, status_code))
        except urllib2.HTTPError as e:
            failure_count += 1
            print("Request #{0}: HTTP Error, Status Code {1}".format(i + 1, e.code))
        except urllib2.URLError as e:
            failure_count += 1
            print("Request #{0}: URL Error, Reason {1}".format(i + 1, e.reason))
        except Exception as e:
            failure_count += 1
            print("Request #{0}: Other Error, {1}".format(i + 1, str(e)))
        finally:
            if 'response' in locals():
                response.close()  # Explicitly close the response object

    print("\nFinished testing.")
    print("Total Successful Requests: {0}".format(success_count))
    print("Total Failed Requests: {0}".format(failure_count))  # Corrected format index here

# URL to be tested
test_url = "http://localhost:8080/"
test_server(test_url)

