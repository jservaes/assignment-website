const API_BASE = "http://localhost:8080";

const userIdInput = document.getElementById("userIdInput");
const loadBtn = document.getElementById("loadBtn");
const itemsGrid = document.getElementById("itemsGrid");
const newItemForm = document.getElementById("newItemForm");

async function fetchItems() {
  const userId = userIdInput.value;
  if (!userId) return;

  const res = await fetch(`${API_BASE}/api/items?userId=${userId}`);
  const data = await res.json();
  renderItems(data);
}

function renderItems(items) {
  itemsGrid.innerHTML = "";
  if (!items.length) {
    itemsGrid.innerHTML = "<p>No items yet. Add one above ✏️</p>";
    return;
  }

  items.forEach(item => {
    const card = document.createElement("div");
    card.className = "item-card";

    const header = document.createElement("div");
    header.className = "item-header";

    const title = document.createElement("div");
    title.className = "item-title";
    title.textContent = item.title;

    const category = document.createElement("div");
    category.className = `item-category category-${item.category}`;
    category.textContent = item.category;

    header.appendChild(title);
    header.appendChild(category);

    const desc = document.createElement("div");
    desc.className = "item-description";
    desc.textContent = item.description || "";

    const meta = document.createElement("div");
    meta.className = "item-meta";

    const due = document.createElement("div");
    due.textContent = "Due: " + new Date(item.dueDate).toLocaleString();

    const badges = document.createElement("div");
    badges.className = "badges";

    if (item.badge === "DUE_SOON") {
      const b = document.createElement("span");
      b.className = "badge badge-due-soon";
      b.textContent = "Due Soon";
      badges.appendChild(b);
    } else if (item.badge === "OVERDUE") {
      const b = document.createElement("span");
      b.className = "badge badge-overdue";
      b.textContent = "OVERDUE!";
      badges.appendChild(b);
    }

    meta.appendChild(due);
    meta.appendChild(badges);

    const footer = document.createElement("div");
    footer.className = "item-footer";

    const doneBtn = document.createElement("button");
    doneBtn.textContent = "Mark Done";
    doneBtn.onclick = () => markDone(item.id);

    footer.appendChild(doneBtn);

    card.appendChild(header);
    if (item.description) card.appendChild(desc);
    card.appendChild(meta);
    card.appendChild(footer);

    itemsGrid.appendChild(card);
  });
}

async function markDone(id) {
  await fetch(`${API_BASE}/api/items/${id}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ status: "Done" })
  });
  // Backend auto-removes item; just refresh
  fetchItems();
}

newItemForm.addEventListener("submit", async (e) => {
  e.preventDefault();
  const userId = parseInt(userIdInput.value, 10);
  const title = document.getElementById("title").value.trim();
  const description = document.getElementById("description").value.trim();
  const category = document.getElementById("category").value;
  const dueDate = document.getElementById("dueDate").value;

  if (!title || !dueDate || !userId) return;

  await fetch(`${API_BASE}/api/items`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ userId, title, description, category, dueDate })
  });

  newItemForm.reset();
  fetchItems();
});

loadBtn.addEventListener("click", fetchItems);

// Auto-load initial userId=1
fetchItems();
