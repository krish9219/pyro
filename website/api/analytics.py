#!/usr/bin/env python3
"""Pyro Analytics API — tracks page views and events"""
import json
import os
import time
from datetime import datetime
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

LOG_FILE = "/projects/pyro/analytics.jsonl"

@app.route("/collect", methods=["POST"])
def collect():
    try:
        data = request.get_json(force=True)
        data['server_ts'] = datetime.now().isoformat()
        data['ip'] = request.remote_addr
        with open(LOG_FILE, 'a') as f:
            f.write(json.dumps(data) + '\n')
    except:
        pass
    return '', 204

@app.route("/stats", methods=["GET"])
def stats():
    if not os.path.exists(LOG_FILE):
        return jsonify({"views": 0, "sessions": 0, "playground_runs": 0})

    views = 0
    sessions = set()
    playground_runs = 0
    pages = {}

    with open(LOG_FILE) as f:
        for line in f:
            try:
                d = json.loads(line)
                if d.get('event') == 'pageview':
                    views += 1
                    pg = d.get('page', '/')
                    pages[pg] = pages.get(pg, 0) + 1
                if d.get('event') == 'playground_run':
                    playground_runs += 1
                if d.get('sid'):
                    sessions.add(d['sid'])
            except:
                pass

    return jsonify({
        "total_views": views,
        "unique_sessions": len(sessions),
        "playground_runs": playground_runs,
        "top_pages": dict(sorted(pages.items(), key=lambda x: -x[1])[:10])
    })

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=9879, debug=False)
