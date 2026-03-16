/* ============================================================
   Pyro Language Website — Main JavaScript
   ============================================================ */
(function () {
  'use strict';

  /* ---------- Mobile Menu ---------- */
  const navToggle = document.querySelector('.nav-toggle');
  const navLinks = document.querySelector('.nav-links');

  if (navToggle && navLinks) {
    navToggle.addEventListener('click', function () {
      navToggle.classList.toggle('open');
      navLinks.classList.toggle('open');
      const expanded = navToggle.getAttribute('aria-expanded') === 'true';
      navToggle.setAttribute('aria-expanded', String(!expanded));
    });

    // Close menu when a link is clicked
    navLinks.querySelectorAll('a').forEach(function (link) {
      link.addEventListener('click', function () {
        navToggle.classList.remove('open');
        navLinks.classList.remove('open');
        navToggle.setAttribute('aria-expanded', 'false');
      });
    });
  }

  /* ---------- Docs Sidebar Toggle (mobile) ---------- */
  const sidebarToggle = document.querySelector('.sidebar-toggle');
  const docsSidebar = document.querySelector('.docs-sidebar');

  if (sidebarToggle && docsSidebar) {
    sidebarToggle.addEventListener('click', function () {
      docsSidebar.classList.toggle('open');
    });
  }

  /* ---------- Tab Switching ---------- */
  document.querySelectorAll('[data-tabs]').forEach(function (tabGroup) {
    const buttons = tabGroup.querySelectorAll('.tab-btn');
    const parentContainer = tabGroup.closest('.tab-container') || tabGroup.parentElement;
    const contents = parentContainer.querySelectorAll('.tab-content');

    buttons.forEach(function (btn) {
      btn.addEventListener('click', function () {
        var target = btn.getAttribute('data-tab');

        buttons.forEach(function (b) { b.classList.remove('active'); });
        btn.classList.add('active');

        contents.forEach(function (c) {
          c.classList.toggle('active', c.getAttribute('data-tab-content') === target);
        });
      });
    });
  });

  /* ---------- Copy to Clipboard ---------- */
  document.addEventListener('click', function (e) {
    var btn = e.target.closest('.copy-btn, .copy-inline');
    if (!btn) return;

    var target = btn.getAttribute('data-copy-target');
    var text;

    if (target) {
      var el = document.querySelector(target);
      text = el ? el.textContent : '';
    } else {
      var pre = btn.closest('pre') || btn.closest('.install-cmd');
      var code = pre ? pre.querySelector('code') : null;
      text = code ? code.textContent : (pre ? pre.textContent : '');
    }

    text = text.trim();

    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(text).then(function () {
        showCopied(btn);
      });
    } else {
      // Fallback
      var ta = document.createElement('textarea');
      ta.value = text;
      ta.style.position = 'fixed';
      ta.style.opacity = '0';
      document.body.appendChild(ta);
      ta.select();
      document.execCommand('copy');
      document.body.removeChild(ta);
      showCopied(btn);
    }
  });

  function showCopied(btn) {
    var orig = btn.textContent;
    btn.textContent = 'Copied!';
    btn.classList.add('copied');
    setTimeout(function () {
      btn.textContent = orig;
      btn.classList.remove('copied');
    }, 1500);
  }

  /* ---------- Smooth Scroll for Anchor Links ---------- */
  document.querySelectorAll('a[href^="#"]').forEach(function (link) {
    link.addEventListener('click', function (e) {
      var hash = this.getAttribute('href');
      if (hash.length <= 1) return;
      var target = document.querySelector(hash);
      if (!target) return;
      e.preventDefault();
      var offset = parseInt(getComputedStyle(document.documentElement).getPropertyValue('--nav-height')) || 64;
      var y = target.getBoundingClientRect().top + window.scrollY - offset - 16;
      window.scrollTo({ top: y, behavior: 'smooth' });
      history.pushState(null, '', hash);
    });
  });

  /* ---------- Intersection Observer for Animations ---------- */
  var fadeEls = document.querySelectorAll('.fade-in');
  if (fadeEls.length && 'IntersectionObserver' in window) {
    var observer = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) {
          entry.target.classList.add('visible');
          observer.unobserve(entry.target);
        }
      });
    }, { threshold: 0.1, rootMargin: '0px 0px -40px 0px' });

    fadeEls.forEach(function (el) { observer.observe(el); });
  }

  /* ---------- Benchmark Bar Animation ---------- */
  var benchBars = document.querySelectorAll('.bench-bar');
  if (benchBars.length && 'IntersectionObserver' in window) {
    var benchObserver = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) {
          entry.target.classList.add('animated');
          benchObserver.unobserve(entry.target);
        }
      });
    }, { threshold: 0.2 });

    benchBars.forEach(function (bar) { benchObserver.observe(bar); });
  }

  /* ---------- Counter Animation ---------- */
  function animateCounter(el) {
    var target = parseInt(el.getAttribute('data-count'), 10);
    if (isNaN(target)) return;
    var duration = 1500;
    var start = 0;
    var startTime = null;

    function step(timestamp) {
      if (!startTime) startTime = timestamp;
      var progress = Math.min((timestamp - startTime) / duration, 1);
      var eased = 1 - Math.pow(1 - progress, 3); // ease-out cubic
      var current = Math.floor(eased * target);
      el.textContent = current.toLocaleString();
      if (progress < 1) requestAnimationFrame(step);
      else el.textContent = target.toLocaleString();
    }

    requestAnimationFrame(step);
  }

  var counters = document.querySelectorAll('[data-count]');
  if (counters.length && 'IntersectionObserver' in window) {
    var counterObserver = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) {
          animateCounter(entry.target);
          counterObserver.unobserve(entry.target);
        }
      });
    }, { threshold: 0.5 });

    counters.forEach(function (el) { counterObserver.observe(el); });
  }

  /* ---------- Category Filter (examples page) ---------- */
  document.querySelectorAll('.filter-btn').forEach(function (btn) {
    btn.addEventListener('click', function () {
      var cat = btn.getAttribute('data-filter');
      var grid = document.querySelector('.examples-grid');
      if (!grid) return;

      document.querySelectorAll('.filter-btn').forEach(function (b) { b.classList.remove('active'); });
      btn.classList.add('active');

      grid.querySelectorAll('.example-card').forEach(function (card) {
        if (cat === 'all' || card.getAttribute('data-category') === cat) {
          card.style.display = '';
        } else {
          card.style.display = 'none';
        }
      });
    });
  });

})();
