from notifier import EmailNotifier
import argparse

ap = argparse.ArgumentParser(description='Email Script')
# -- summary_len: 100, 200, 400
ap.add_argument('-p', '--payload', type=str, help='Folder to be sent', required=True)

args = ap.parse_args()

notifier = EmailNotifier(args.payload, args.payload)
notifier.send_payload()
