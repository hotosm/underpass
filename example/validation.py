import underpass as u
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
            print(validateOsmChange(data, args.check))
    else:
        print("Usage: python validation.py -f <file> -c <check>")
        print("Example: python ../example/validation.py -f ../testsuite/testdata/validation/building-tag.osc -c building")
if __name__ == "__main__":
    main()
