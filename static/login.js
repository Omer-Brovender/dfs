const loginFormDiv = document.getElementById("login");
const signupFormDiv = document.getElementById("signup");
const showSignup = document.getElementById("show-signup");
const showLogin = document.getElementById("show-login");
const signupForm = document.getElementById("signup-form");
const loginForm = document.getElementById("login-form");

const successBox = document.getElementById("success-box");
const errorBox   = document.getElementById("error-box");

function hideBoxes() {
    successBox.classList.remove("active");
    errorBox.classList.remove("active");
}

showSignup.addEventListener("click", (e) => {
    e.preventDefault();
    loginFormDiv.classList.remove("active");
    signupFormDiv.classList.add("active");
});

showLogin.addEventListener("click", (e) => {
    e.preventDefault();
    signupFormDiv.classList.remove("active");
    loginFormDiv.classList.add("active");
});

signupForm.addEventListener('submit', async function (e) {
    e.preventDefault();
    const formData = new FormData(signupForm);
    console.log(formData.values());
    const response = await fetch("/api/signup", {
        method: "POST",
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(Object.fromEntries(formData)),
    });

    const result = await response.text();
    hideBoxes();
    if (response.ok) {
        successBox.classList.add("active");
        successBox.innerHTML = result;
    } else {
        errorBox.classList.add("active");
        errorBox.innerHTML = result;
    }
    console.log(result);
});

loginForm.addEventListener('submit', async function (e) {
    e.preventDefault();
    const formData = new FormData(loginForm);
    console.log(formData.values());
    const response = await fetch("/api/login", {
        method: "POST",
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(Object.fromEntries(formData)),
    });

    hideBoxes();
    
    if (response.ok) {
        const result = await response.json();
        console.log("sessionID=" + result.session + "; HttpOnly; SameSite=Strict");
        document.cookie = "sessionID=" + result.session + "; SameSite=Strict";
        location.reload();
    } else {
        textResult = await response.text();
        errorBox.classList.add("active");
        errorBox.innerHTML = textResult;
        console.error("Login failed");
    }
});
