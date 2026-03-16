// Fetch stats from API and display on home page
var statsEl = document.getElementById('stats');
if (statsEl) {
    fetch('/api/stats')
        .then(function(r) { return r.json(); })
        .then(function(data) {
            // data is an array: ["visitors", 1024, "projects", 5, ...]
            statsEl.innerHTML =
                '<div class="stat-box"><div class="stat-num">' + data.visitors + '</div><div class="stat-label">Visitors</div></div>' +
                '<div class="stat-box"><div class="stat-num">' + data.projects + '</div><div class="stat-label">Projects</div></div>' +
                '<div class="stat-box"><div class="stat-num">' + data.languages.length + '</div><div class="stat-label">Languages</div></div>';
        })
        .catch(function() {});
}

// Highlight current nav link
var links = document.querySelectorAll('nav a');
for (var i = 0; i < links.length; i++) {
    if (links[i].getAttribute('href') === window.location.pathname) {
        links[i].style.color = '#ff6b35';
    }
}
