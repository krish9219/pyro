// Pyro Analytics — lightweight page view + event tracking
(function() {
    var API = '/pyro-lang/api/analytics';
    var sid = sessionStorage.getItem('pyro_sid');
    if (!sid) { sid = Math.random().toString(36).substr(2, 9); sessionStorage.setItem('pyro_sid', sid); }

    function track(event, data) {
        try {
            navigator.sendBeacon(API, JSON.stringify({
                event: event,
                page: location.pathname,
                sid: sid,
                ts: Date.now(),
                ref: document.referrer,
                ua: navigator.userAgent,
                data: data || {}
            }));
        } catch(e) {}
    }

    // Track page view
    track('pageview');

    // Track playground runs
    window.pyroTrack = track;
})();
