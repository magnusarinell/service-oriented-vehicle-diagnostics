# Multi-stage build for frontend (Vite + Vue 3)
FROM node:22-alpine AS build
WORKDIR /app

COPY SovdClient/package.json SovdClient/package-lock.json* ./
RUN npm ci

COPY SovdClient/ .
RUN npm run build

# ── Runtime: nginx ────────────────────────────────────────────────────────────
FROM nginx:1.25-alpine
COPY --from=build /app/dist /usr/share/nginx/html
# Proxy /api to sovd-server
COPY docker/nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
