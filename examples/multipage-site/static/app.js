// Highlight active nav link
var links = document.querySelectorAll('nav a');
for (var i = 0; i < links.length; i++) {
    if (links[i].getAttribute('href') === window.location.pathname) {
        links[i].classList.add('active');
    }
}

// Load stats on home page
var statsEl = document.getElementById('stats');
if (statsEl) {
    fetch('/api/stats')
        .then(function(r) { return r.json(); })
        .then(function(data) {
            statsEl.innerHTML =
                '<div class="stat-box"><div class="stat-num">' + data.visitors + '</div><div class="stat-label">Visitors</div></div>' +
                '<div class="stat-box"><div class="stat-num">' + data.projects + '</div><div class="stat-label">Projects</div></div>';
        })
        .catch(function() {});
}
