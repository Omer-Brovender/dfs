const upload = document.querySelector(".upload-button");
const input = document.getElementById("upload-input");
const fileGrid = document.querySelector(".file-grid")
const contextMenu = document.querySelector(".file-context")
const downloadButton = document.querySelector(".download-button")

upload.addEventListener("click", (e) => {
    e.preventDefault();
    input.click();
});

input.addEventListener("change", (e) => {
    const file = input.files[0];
    if (!file) return;

    const formData = new FormData();
    formData.append("file", file);

    const request = new XMLHttpRequest();
    request.open('POST', '/api/upload', true);

    request.onload = () => {
        console.log(request.responseText);
    };

    request.onerror = () => {
        console.log("Error sending request.");
    };

    request.send(formData);
});

document.addEventListener("click", (e) => {
    if (!contextMenu.contains(e.target)) {
        contextMenu.classList.add("inactive");
    }
});

const request = new XMLHttpRequest();
request.responseType = "json";
request.open('GET', '/api/files', true);

var fileID = -1;

request.onload = () => {
    let files = request.response;
    console.log(request.response);

    for (const id in files) {
        let icon = document.createElement("p");
        icon.innerHTML = "📄"
        icon.classList.add("icon")

        let bg = document.createElement("div");
        bg.classList.add("icon-background");

        let title = document.createElement("p");
        title.innerHTML = "[" + id.toString() + "] " + files[id];
        title.classList.add("name");

        let file = document.createElement("div");
        file.classList.add("file");

        file.addEventListener("contextmenu", (e) => {
            e.preventDefault();
            fileID = id;

            contextMenu.style.top = `${e.pageY}px`;
            contextMenu.style.left = `${e.pageX}px`;
            contextMenu.classList.remove("inactive");
        });

        bg.appendChild(icon);
        file.appendChild(bg);
        file.appendChild(title);
        fileGrid.appendChild(file);
    }
};

downloadButton.addEventListener("click", () => {
    if (fileID >= 0) {
        const link = document.createElement("a");
        link.href = `/api/download/${fileID}`;
        link.download = "";
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
    }
    contextMenu.classList.add("inactive");
});

request.onerror = () => {
    console.log("Error sending request.");
};

request.send();

/*
<div class="file">
    <div class="icon-background">
        <p class="icon">📄</p>
    </div>
    <p class="name">Document</p>
</div>
*/