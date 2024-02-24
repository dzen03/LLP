import json
import subprocess
import time

with open('records.json') as f:
    test_data = json.load(f)

with open('server/test.bin', 'w'):
    pass

print(test_data)

# serv = subprocess.Popen(['./client/lab3-server', 'test.bin'],
#                           stdout=subprocess.PIPE)

for line in test_data:
    insert = """
insert {{
    customer {{
        name = "{0}"
        purchased {{
            country = "{1}"
            zip = {2}
        }}
    }}
}}
""".format(line['name'], line['country'], line['zip'])
    with open('test_data', 'w') as f:
        print(insert, file=f)
    output = subprocess.Popen(['./client/lab3-client', 'test_data'],
                              stdout=subprocess.PIPE)

    output.wait()


# serv.kill()


