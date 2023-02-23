from underpass import underpass as u
import argparse

def validateOsmChange(xml: str, check: str):
    validator = u.Validate()
    return validator.checkOsmChange(xml, check)

def main():
    args = argparse.ArgumentParser()
    args.add_argument("--file", "-f", help="OsmChange file", type=str, default=None)
    args.add_argument("--check", "-c", help="Check (ex: building)", type=str, default=None)
    args = args.parse_args()

    if args.file and args.check:
        with open(args.file, 'r') as file:
            data = file.read().rstrip()
            # print(validateOsmChange(data, args.check))
            validateOsmChange(data, args.check)
    else:
        print("Usage: python validaiton.py -f mychange.osm -c building")
        print("Example: python ../example/validation.py -f ../testsuite/testdata/stats/107235440.xml -c highway")
if __name__ == "__main__":
    main()
