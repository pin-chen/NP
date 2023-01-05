#! /usr/bin/env python3
import sys
import time
import html

def output_shell(session, content):
    content = html.escape(content)
    content = content.replace('\n', '&NewLine;')
    print(f"<script>document.getElementById('{session}').innerHTML += '{content}';</script>")
    sys.stdout.flush()

def output_command(session, content):
    content = html.escape(content)
    content = content.replace('\n', '&NewLine;')
    print(f"<script>document.getElementById('{session}').innerHTML += '<b>{content}</b>';</script>")
    sys.stdout.flush()

print('Content-type: text/html', end='\r\n\r\n')

print(
'''
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>NP Project 3 Sample Console</title>
    <link
      rel="stylesheet"
      href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
      integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
      crossorigin="anonymous"
    />
    <link
      href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
      rel="stylesheet"
    />
    <link
      rel="icon"
      type="image/png"
      href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
    />
    <style>
      * {
        font-family: 'Source Code Pro', monospace;
        font-size: 1rem !important;
      }
      body {
        background-color: #212529;
      }
      pre {
        color: #cccccc;
      }
      b {
        color: #01b468;
      }
    </style>
  </head>
  <body>
    <table class="table table-dark table-bordered">
      <thead>
        <tr>
          <th scope="col">nplinux1.cs.nctu.edu.tw:1234</th>
          <th scope="col">nplinux2.cs.nctu.edu.tw:5678</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><pre id="s0" class="mb-0"></pre></td>
          <td><pre id="s1" class="mb-0"></pre></td>
        </tr>
      </tbody>
    </table>
  </body>
</html>
'''
)

output_shell('s0', '****************************************\n')
output_shell('s0', '** Welcome to the information server. **\n')
output_shell('s0', '****************************************\n')
output_shell('s0', "*** User '(no name)' entered from 140.113.235.234:59894. ***\n")
output_shell('s0', '% ')

output_shell('s1', '****************************************\n')
output_shell('s1', '** Welcome to the information server. **\n')
output_shell('s1', '****************************************\n')
output_shell('s1', "*** User '(no name)' entered from 140.113.235.234:59895. ***\n")
output_shell('s1', '% ')

time.sleep(0.5)
output_command('s0', 'ls\n')
time.sleep(0.5)
output_shell('s0', 'bin\n')
time.sleep(0.5)
output_shell('s0', 'test.html\n')
time.sleep(0.5)
output_shell('s0', '% ')


time.sleep(0.5)
output_command('s1', 'ls |1\n')
time.sleep(0.5)
output_command('s0', 'exit\n')
time.sleep(0.5)
output_shell('s1', '% ')
time.sleep(0.5)
output_command('s1', 'number\n')
time.sleep(0.5)
output_shell('s1', '   1 bin\n')
time.sleep(0.5)
output_shell('s1', '   2 test.html\n')
time.sleep(0.5)
output_shell('s1', '% ')
time.sleep(0.5)
output_command('s1', 'exit\n')
time.sleep(0.5)
